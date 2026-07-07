#include "Servo.h"
#include "Servo_Cfg.h"
#include "robot_config.h"

#include "esp_log.h"
#include <string.h>

static const char *TAG = "SERVO";

/* ---- 内部状态 ---- */
static pca9685_t *pca_dev = NULL;
static uint16_t servo_ticks[SERVO_COUNT];      /* PWM 比较值缓冲 */
static bool servo_enabled = true;

/* ---- 舵机方向校正查找表 (从 Servo_Cfg.h 的宏编译为运行时数据) ---- */

static const int8_t LegDir[NUM_LEGS][JOINTS_PER_LEG] = {
    /* Coxa,    Femur,    Tibia */
    { LEG_RR_COXA_DIR,  LEG_RR_FEMUR_DIR,  LEG_RR_TIBIA_DIR  },  /* RR */
    { LEG_RM_COXA_DIR,  LEG_RM_FEMUR_DIR,  LEG_RM_TIBIA_DIR  },  /* RM */
    { LEG_RF_COXA_DIR,  LEG_RF_FEMUR_DIR,  LEG_RF_TIBIA_DIR  },  /* RF */
    { LEG_LR_COXA_DIR,  LEG_LR_FEMUR_DIR,  LEG_LR_TIBIA_DIR  },  /* LR */
    { LEG_LM_COXA_DIR,  LEG_LM_FEMUR_DIR,  LEG_LM_TIBIA_DIR  },  /* LM */
    { LEG_LF_COXA_DIR,  LEG_LF_FEMUR_DIR,  LEG_LF_TIBIA_DIR  },  /* LF */
};

/* =================================================================== */

/**
 * 脉宽 (us) → PCA9685 PWM 计数值
 * 50Hz → 20ms 周期 → 20000us → 4096 ticks
 */
static inline uint16_t us_to_ticks(uint16_t us)
{
    return (uint16_t)(((uint32_t)us * 4096UL) / 20000UL);
}

/**
 * 角度 (0.1 deg) → 脉宽 (us)
 */
static inline uint16_t angle_to_us(int16_t angle)
{
    /* 限幅 */
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    /*
     * 线性映射:
     *   SERVO_ANGLE_MIN → SERVO_MIN_US
     *   SERVO_ANGLE_MAX → SERVO_MAX_US
     *   0 (center)     → SERVO_CENTER_US
     *
     * 角度范围: 1800 (from -900 to +900)
     * 脉宽范围: 2000 (from 500 to 2500)
     */
    int32_t us = SERVO_CENTER_US +
                 (int32_t)angle * (SERVO_MAX_US - SERVO_MIN_US) / 1800;

    return (uint16_t)us;
}

/* =================================================================== */

void servo_init(pca9685_t *pca)
{
    pca_dev = pca;

    /* 初始化缓冲区：全部设为中立点 */
    uint16_t center_tick = us_to_ticks(SERVO_CENTER_US);
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        servo_ticks[i] = center_tick;
    }

    /* 立即写入 PCA9685，将所有舵机置于中立点 */
    servo_commit();

    ESP_LOGI(TAG, "Servo subsystem initialized (%d channels)", SERVO_COUNT);
}

void servo_set_angle(uint8_t index, int16_t angle)
{
    if (index >= SERVO_COUNT || !pca_dev) return;

    /* 应用方向校正 */
    uint8_t leg = index / JOINTS_PER_LEG;
    uint8_t joint = index % JOINTS_PER_LEG;
    angle = angle * LegDir[leg][joint];

    uint16_t us = angle_to_us(angle);
    servo_ticks[index] = us_to_ticks(us);
}

void servo_commit(void)
{
    if (!pca_dev || !servo_enabled) return;

    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        pca9685_set_pwm(pca_dev, i, 0, servo_ticks[i]);
    }
}

void servo_release(void)
{
    if (!pca_dev) return;

    /* 将所有通道设为 0 占空比 */
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        pca9685_set_pwm(pca_dev, i, 0, 0);
    }
    servo_enabled = false;

    ESP_LOGI(TAG, "All servos released");
}
