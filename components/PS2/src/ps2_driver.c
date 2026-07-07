#include "ps2_driver.h"
#include "platform.h"

#include "esp_log.h"
#include "driver/spi_master.h"
#include <string.h>

static const char *TAG = "PS2";

/* ---- 内部 SPI 设备句柄 (由 ps2_init 设置) ---- */
static spi_device_handle_t spi_dev = NULL;

/* ---- PS2 配置序列命令 ---- */

/* 进入配置模式 */
static const uint8_t CMD_ENTER_CONFIG[] = {
    0x01, 0x43, 0x00, 0x01, 0x00
};
/* 启用模拟模式 + 锁定 ANALOG 按钮 */
static const uint8_t CMD_ANALOG_MODE[] = {
    0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00
};
/* 退出配置模式并保存 */
static const uint8_t CMD_EXIT_CONFIG[] = {
    0x01, 0x43, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A
};
/* 标准轮询命令 (9-byte, 无振动) */
static const uint8_t POLL_CMD[] = {
    0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* ---- 内部 SPI 传输 ---- */

/**
 * PS2 全双工传输: 发送 tx 的每个字节的同时接收 rx。
 * 由于 SPI_DEVICE_BIT_LSBFIRST 已配置, 字节内的位序由硬件处理。
 */
static esp_err_t ps2_transfer(const uint8_t *tx, uint8_t *rx, size_t len)
{
    /*
     * ESP32 SPI 的 BIT_LSBFIRST 只反转位序, LSB first 要求字节先发 LSB。
     * 但 PS2 的 LSB first 是在字节级别: 先发送字节的低位。
     * 由于 SPI 本身是 8-bit 传输, 使用 BIT_LSBFIRST 标志是正确的。
     */
    spi_transaction_t trans = {
        .length = len * 8,
        .rxlength = len * 8,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    return spi_device_transmit(spi_dev, &trans);
}

/* ---- 解析 ---- */

static void ps2_parse_packet(const uint8_t *rx, PS2State_t *state)
{
    /* RX[1] = ID (0x41=数字, 0x73=模拟红LED, 0x79=模拟+压力) */
    /* RX[2] = 0x5A 数据有效标记 */

    state->analog_mode = ((rx[1] & 0xF0) == 0x70);
    state->prev_buttons = state->buttons;

    if (rx[2] == 0x5A) {
        /* B1 (RX[3]): SELECT, L3, R3, START, UP, RIGHT, DOWN, LEFT */
        /* B2 (RX[4]): SQUARE, CROSS, CIRCLE, TRIANGLE, R1, L1, R2, L2 */

        /*
         * 按键是 active LOW。
         * 转换: bit=0 (按下) → mask bit=1
         *        bit=1 (松开) → mask bit=0
         *
         * B1 bit 0 = SELECT → PSB_SELECT (0x0001)
         * B1 bit 1 = L3     → PSB_L3     (0x0002)
         * B1 bit 2 = R3     → PSB_R3     (0x0004)
         * B1 bit 3 = START  → PSB_START  (0x0008)
         * B1 bit 4 = UP     → PSB_PAD_UP (0x0010)
         * B1 bit 5 = RIGHT  → PSB_PAD_RIGHT
         * B1 bit 6 = DOWN   → PSB_PAD_DOWN
         * B1 bit 7 = LEFT   → PSB_PAD_LEFT
         */
        uint16_t b1 = ~rx[3] & 0xFF;
        /* B2 bit 0 = L2      → PSB_L2 (0x0100)
         * B2 bit 1 = R2      → PSB_R2 (0x0200)
         * B2 bit 2 = L1      → PSB_L1 (0x0400)
         * B2 bit 3 = R1      → PSB_R1 (0x0800)
         * B2 bit 4 = TRIANGLE→ PSB_TRIANGLE
         * B2 bit 5 = CIRCLE  → PSB_CIRCLE
         * B2 bit 6 = CROSS   → PSB_CROSS
         * B2 bit 7 = SQUARE  → PSB_SQUARE
         */
        uint16_t b2 = ~rx[4] & 0xFF;

        state->buttons = (b1) | ((b2 << 8) & 0xFF00);

        /* 摇杆轴值 */
        state->axes[5] = rx[5];  /* RX */
        state->axes[6] = rx[6];  /* RY */
        state->axes[7] = rx[7];  /* LX */
        state->axes[8] = rx[8];  /* LY */

        state->connected = true;
    } else {
        state->connected = false;
    }
}

/* ==================== 公开 API ==================== */

bool ps2_init(spi_device_handle_t spi_handle)
{
    uint8_t rx[9] = {0};
    uint8_t tx[9] = {0};

    ESP_LOGI(TAG, "Initializing PS2 controller...");

    spi_dev = spi_handle;

    /* Step 1: 短轮询 5 次, 确认连接 */
    bool found = false;
    for (int i = 0; i < 5; i++) {
        memcpy(tx, POLL_CMD, 5);
        ps2_transfer(tx, rx, 5);
        if (rx[2] == 0x5A) {
            found = true;
            break;
        }
        platform_delay(10);
    }

    if (!found) {
        ESP_LOGW(TAG, "PS2 controller not detected (no valid response)");
        return false;
    }

    ESP_LOGI(TAG, "PS2 controller detected (ID=0x%02X), entering analog mode...", rx[1]);

    /* Step 2: 进入配置模式 */
    memset(tx, 0, 9);
    memcpy(tx, CMD_ENTER_CONFIG, sizeof(CMD_ENTER_CONFIG));
    ps2_transfer(tx, rx, 9);
    platform_delay(10);

    /* Step 3: 锁定为模拟模式 (防止 ANALOG 按钮切换) */
    memset(tx, 0, 9);
    memcpy(tx, CMD_ANALOG_MODE, sizeof(CMD_ANALOG_MODE));
    ps2_transfer(tx, rx, 9);
    platform_delay(10);

    /* Step 4: 退出配置模式 */
    memset(tx, 0, 9);
    memcpy(tx, CMD_EXIT_CONFIG, sizeof(CMD_EXIT_CONFIG));
    ps2_transfer(tx, rx, 9);
    platform_delay(10);

    /* 验证模式 */
    PS2State_t state;
    for (int i = 0; i < 3; i++) {
        ps2_poll(&state);
        platform_delay(10);
    }

    if (state.analog_mode) {
        ESP_LOGI(TAG, "PS2 controller ready (analog mode, red LED)");
    } else {
        ESP_LOGW(TAG, "PS2 controller in digital mode (green LED), analog stick will not work");
    }

    return true;
}

bool ps2_poll(PS2State_t *state)
{
    uint8_t rx[9] = {0};
    uint8_t tx[9];

    memcpy(tx, POLL_CMD, sizeof(POLL_CMD));
    esp_err_t err = ps2_transfer(tx, rx, 9);

    if (err != ESP_OK) {
        state->connected = false;
        return false;
    }

    ps2_parse_packet(rx, state);
    return state->connected;
}

/* ---- 按钮/摇杆辅助函数 ---- */

bool ps2_button(uint16_t buttons, uint16_t mask)
{
    return (buttons & mask) != 0;
}

bool ps2_button_pressed(const PS2State_t *state, uint16_t mask)
{
    uint16_t delta = state->buttons ^ state->prev_buttons;
    return (delta & state->buttons & mask) != 0;
}

bool ps2_button_released(const PS2State_t *state, uint16_t mask)
{
    uint16_t delta = state->buttons ^ state->prev_buttons;
    return (delta & state->prev_buttons & mask) != 0;
}

bool ps2_new_button_state(const PS2State_t *state)
{
    return state->buttons != state->prev_buttons;
}

uint8_t ps2_analog(const PS2State_t *state, uint8_t axis)
{
    if (axis < 5 || axis > 8) return 128; /* 未知轴, 返回中点 */
    return state->axes[axis];
}
