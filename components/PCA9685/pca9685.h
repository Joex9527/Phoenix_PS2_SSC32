#pragma once

#include <stdint.h>
#include "esp_err.h"

/* Forward declarations to avoid pulling in esp_driver_i2c dependency */
typedef struct i2c_master_bus_t *i2c_master_bus_handle_t;
typedef struct i2c_master_dev_t *i2c_master_dev_handle_t;

/* ==================== PCA9685 寄存器地址 ==================== */

#define PCA9685_MODE1       0x00
#define PCA9685_MODE2       0x01
#define PCA9685_PRESCALE    0xFE

#define LED0_ON_L           0x06
#define LED0_ON_H           0x07
#define LED0_OFF_L          0x08
#define LED0_OFF_H          0x09

#define ALL_LED_ON_L        0xFA
#define ALL_LED_ON_H        0xFB
#define ALL_LED_OFF_L       0xFC
#define ALL_LED_OFF_H       0xFD

/* I2C 时钟频率 */
#define PCA9685_I2C_FREQ_HZ 400000

/* ==================== PCA9685 设备句柄 ==================== */

typedef struct {
    i2c_master_dev_handle_t dev;
    uint8_t address;
} pca9685_t;

/* ==================== API ==================== */

/**
 * @brief 初始化 PCA9685 设备
 * @param pca     设备句柄
 * @param bus     I2C 总线句柄（由 board_i2c_init 创建）
 * @param address I2C 从机地址（默认 0x40）
 */
esp_err_t pca9685_init(
    pca9685_t *pca,
    i2c_master_bus_handle_t bus,
    uint8_t address
);

/**
 * @brief 设置 PWM 频率
 * @param pca     设备句柄
 * @param freq_hz 目标频率（舵机典型值 50Hz）
 */
esp_err_t pca9685_set_pwm_freq(pca9685_t *pca, float freq_hz);

/**
 * @brief 设置单通道 PWM 占空比
 * @param pca     设备句柄
 * @param ch      通道号 (0-15)
 * @param on      ON 计数 (0-4095)
 * @param off     OFF 计数 (0-4095)
 */
esp_err_t pca9685_set_pwm(pca9685_t *pca, uint8_t ch, uint16_t on, uint16_t off);

/**
 * @brief 进入睡眠模式（释放所有舵机）
 */
esp_err_t pca9685_sleep(pca9685_t *pca);

/**
 * @brief 唤醒并恢复 PWM 输出
 */
esp_err_t pca9685_wake(pca9685_t *pca);
