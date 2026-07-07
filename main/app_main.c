#include "board.h"
#include "platform.h"
#include "Phoenix.h"
#include "Gait.h"
#include "ps2_driver.h"
#include "ps2_control.h"
#include "serial_protocol.h"
#include "pca9685.h"
#include "Servo.h"
#include "robot_config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

/* ---- 全局设备句柄 ---- */
static pca9685_t         pca_dev;
static ControlState_t    control_state;
static PS2State_t        ps2_state;
static SerialProto_t     serial_proto;
static SerialInput_t     serial_input;

/* ---- 初始化 ---- */

static void system_init(void)
{
    /* 1. 板级外设 */
    board_init();

    /* 2. PCA9685 PWM 驱动 */
    ESP_ERROR_CHECK(pca9685_init(&pca_dev, i2c_bus_handle, 0x40));
    ESP_ERROR_CHECK(pca9685_set_pwm_freq(&pca_dev, 50.0f));

    /* 3. 舵机子系统 */
    servo_init(&pca_dev);

    /* 4. 控制状态 */
    control_state_init(&control_state);
    GaitSelect(GAIT_TRIPOD_6);

    /* 5. PS2 手柄 (非阻塞) */
    bool ps2_ok = ps2_init(ps2_spi_handle);
    if (ps2_ok) {
        ESP_LOGI(TAG, "PS2 controller connected");
    } else {
        ESP_LOGW(TAG, "PS2 controller NOT connected — use serial host instead");
    }

    /* 6. 上位机串口协议 */
    serial_proto_init(&serial_proto, DEBUG_UART_NUM);

    ESP_LOGI(TAG, "System initialization complete");
}

/* ---- 输入处理 ---- */

static void input_update(void)
{
    /* A. 上位机协议 (高优先级) */
    serial_proto_poll(&serial_proto, &serial_input);

    if (serial_input.new_data) {
        /*
         * 上位机直接下发舵机角度 (deg×100)。
         * 绕过 Phoenix 运动学 — 直接写入 servo。
         */
        for (int i = 0; i < NUM_SERVOS; i++) {
            int16_t raw = serial_input.angles_deg100[i];
            if (raw == 0x7FFF) continue;  /* sentinel: 不更新 */
            servo_set_angle(i, raw / 10); /* deg×100 → deg×10 */
        }
        servo_commit();

        if (!serial_input.servos_enabled) {
            control_state.RobotOn = false;
        }
    }

    if (serial_proto_has_control(&serial_proto)) {
        /* 上位机控制中, 不读取 PS2 */
        return;
    }

    /* B. PS2 手柄 */
    if (ps2_state.connected || ps2_init(ps2_spi_handle)) {
        if (ps2_poll(&ps2_state)) {
            ps2_update_control(&control_state, &ps2_state);
        } else {
            /* 手柄断开 */
            ps2_state.connected = false;
            /* 返回默认姿态 */
            control_state_init(&control_state);
            ESP_LOGW(TAG, "PS2 disconnected");
        }
    }
}

/* ---- Phoenix 主控制任务 ---- */

static void phoenix_task(void *arg)
{
    ESP_LOGI(TAG, "Phoenix control task started");

    TickType_t last_wake = xTaskGetTickCount();

    while (1) {
        /* 1. 处理输入 */
        input_update();

        /* 2. 运行 Phoenix 主循环 */
        if (control_state.RobotOn && !serial_proto_has_control(&serial_proto)) {
            PhoenixLoop(&control_state);
        }

        /* 3. 延时到下一帧 */
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(control_state.GaitSpeed));
    }
}

/* =================================================================== */

void app_main(void)
{
    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, "  Phoenix Hexapod ESP32-S3");
    ESP_LOGI(TAG, "  PCA9685 + PS2 + Serial Host");
    ESP_LOGI(TAG, "=============================================");

    system_init();

    /* 创建 Phoenix 主循环任务 (高优先级, 8KB 栈) */
    xTaskCreate(phoenix_task, "phoenix", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "Ready.");
}
