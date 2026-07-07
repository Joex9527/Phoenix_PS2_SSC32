#include "serial_protocol.h"
#include "platform.h"
#include "robot_config.h"

#include "esp_log.h"
#include "driver/uart.h"
#include <string.h>

static const char *TAG = "SERPROTO";

/* ---- 超时 (ms), 超出后释放控制权 ---- */
#define SP_CONTROL_TIMEOUT_MS   3000

/* ---- 内部 ---- */
static int sp_uart_num = 0;

/* =================================================================== */

void serial_proto_init(SerialProto_t *proto, int uart_num)
{
    memset(proto, 0, sizeof(*proto));
    proto->state = SP_IDLE;
    proto->has_control = false;
    proto->last_cmd_time = 0;
    sp_uart_num = uart_num;

    ESP_LOGI(TAG, "Serial protocol parser initialized");
}

/* ---- 字节级帧解析 ---- */

static void sp_process_byte(SerialProto_t *proto, SerialInput_t *input, uint8_t byte)
{
    switch (proto->state) {

    case SP_IDLE:
        if (byte == SP_HEADER1) {
            proto->state = SP_GOT_FE1;
        }
        break;

    case SP_GOT_FE1:
        if (byte == SP_HEADER2) {
            proto->state = SP_GOT_FE2;
        } else {
            proto->state = SP_IDLE;     /* 不是连续两个 0xFE, 重新开始 */
        }
        break;

    case SP_GOT_FE2:
        proto->length = byte;
        proto->cmd = 0;
        proto->payload_idx = 0;
        if (proto->length > 0 && proto->length <= SP_MAX_PAYLOAD + 1) {
            proto->state = SP_IN_PAYLOAD;
        } else {
            ESP_LOGW(TAG, "Invalid length: %d", proto->length);
            proto->state = SP_IDLE;
        }
        break;

    case SP_GOT_LENGTH:
        /* 保留状态, 当前未使用 (长度在 SP_GOT_FE2 中处理) */
        proto->state = SP_IDLE;
        break;

    case SP_IN_PAYLOAD:
        if (proto->payload_idx == 0) {
            /* 第一个 payload 字节 = 命令码 */
            proto->cmd = byte;
        } else {
            /* 后续字节 = 参数 */
            uint8_t idx = proto->payload_idx - 1;
            if (idx < SP_MAX_PAYLOAD) {
                proto->payload[idx] = byte;
            }
        }
        proto->payload_idx++;

        /* payload_idx 包含命令码, 所以 payload 总长度 = payload_idx */
        if (proto->payload_idx >= proto->length) {
            /* 下一个字节应该是 END marker */
            proto->state = SP_IDLE;     /* END 在下一个 poll 周期检查 */
            /* 注意: 我们需要在调用者处检查 0xFA */
        }
        break;
    }
}

/* ---- 命令分发 ---- */

static void sp_dispatch(SerialProto_t *proto, SerialInput_t *input)
{
    proto->last_cmd_time = platform_millis();
    proto->has_control = true;

    int16_t *angles = input->angles_deg100;

    switch (proto->cmd) {

    case SP_CMD_POWER_ON:
        input->servos_enabled = true;
        ESP_LOGI(TAG, "CMD: Power ON");
        break;

    case SP_CMD_SET_ALL:
        /* 0x22: payload = [A1_H A1_L ... A18_H A18_L] [speed%]
         *   18 × 2 = 36 bytes + 1 speed byte = 37 bytes
         *   或 36 bytes (无 speed)
         */
        {
            uint8_t count = (proto->length - 1) / 2;  /* 舵机数量 */
            if (count > NUM_SERVOS) count = NUM_SERVOS;

            for (uint8_t i = 0; i < count; i++) {
                uint16_t raw_u16 = (uint16_t)((proto->payload[i*2] << 8) | proto->payload[i*2 + 1]);
                int16_t raw = (int16_t)raw_u16;  /* 有符号 16-bit: 原始协议中角度 = deg*100 */
                angles[i] = raw;
            }
            input->new_data = true;
        }
        break;

    case SP_CMD_SET_SINGLE:
        /* 0x21: payload = [servo#] [angle_H] [angle_L] [speed%] */
        if (proto->length >= 4) {
            uint8_t servo_id = proto->payload[0];
            uint16_t raw_u16 = (uint16_t)((proto->payload[1] << 8) | proto->payload[2]);
            int16_t raw = (int16_t)raw_u16;

            if (servo_id < NUM_SERVOS) {
                angles[servo_id] = raw;
                /* 标记该舵机为非零, 仅更新单个舵机 */
                for (int i = 0; i < NUM_SERVOS; i++) {
                    if (i != servo_id) angles[i] = 0x7FFF; /* sentinel: 不更新 */
                }
                input->new_data = true;
            }
        }
        break;

    case SP_CMD_READ_ANGLES:
        /* 0x20: 读取当前角度, 调用 serial_proto_send_angles 响应 */
        break;

    case SP_CMD_TORQUE_ON:
        if (proto->length >= 2) {
            input->servos_enabled = true;
            ESP_LOGI(TAG, "CMD: Torque ON servo %d", proto->payload[0]);
        }
        break;

    case SP_CMD_TORQUE_OFF:
        if (proto->length >= 2) {
            ESP_LOGI(TAG, "CMD: Torque OFF servo %d", proto->payload[0]);
        }
        break;

    default:
        ESP_LOGW(TAG, "Unknown command: 0x%02X", proto->cmd);
        break;
    }
}

/* =================================================================== */

void serial_proto_poll(SerialProto_t *proto, SerialInput_t *input)
{
    uint8_t buf[SP_RX_BUF_SIZE];
    int len;

    /* 超时检测: 长时间未收到命令, 释放控制权 */
    if (proto->has_control) {
        uint32_t now = platform_millis();
        if (now - proto->last_cmd_time > SP_CONTROL_TIMEOUT_MS) {
            proto->has_control = false;
            ESP_LOGI(TAG, "Host control timeout, releasing");
        }
    }

    input->new_data = false;

    while ((len = uart_read_bytes(sp_uart_num, buf, sizeof(buf), 0)) > 0) {
        for (int i = 0; i < len; i++) {
            uint8_t byte = buf[i];

            /* 检查是否为 END marker */
            if (byte == SP_END && proto->state == SP_IDLE) {
                /* 帧结束 → 分发命令 */
                sp_dispatch(proto, input);
                continue;
            }

            sp_process_byte(proto, input, byte);
        }
    }
}

void serial_proto_send_angles(const int16_t *angles)
{
    /* 响应格式: FE FE [length] 20 [A1_H A1_L ... A18_H A18_L] FA
     * length = 1 (cmd) + 36 (18×2) + 1 (end) - however length counts from after length byte to FA exclusive
     * Actually: length = number of bytes from length byte to FA (exclusive) = cmd(1) + angles(36) = 37
     */

    uint8_t buf[SP_TX_BUF_SIZE];
    uint8_t idx = 0;

    buf[idx++] = SP_HEADER1;
    buf[idx++] = SP_HEADER2;
    buf[idx++] = 37;                            /* length: 1(cmd) + 36(18 angles) */
    buf[idx++] = SP_CMD_READ_ANGLES;           /* command */

    for (int i = 0; i < NUM_SERVOS; i++) {
        int16_t raw = angles[i];               /* units: deg*10 */
        /* Convert deg*10 to deg*100 (protocol expects deg*100) */
        int16_t encoded = raw * 10;
        buf[idx++] = (encoded >> 8) & 0xFF;
        buf[idx++] = encoded & 0xFF;
    }

    buf[idx++] = SP_END;

    uart_write_bytes(sp_uart_num, (const char *)buf, idx);
}

bool serial_proto_has_control(const SerialProto_t *proto)
{
    return proto->has_control;
}
