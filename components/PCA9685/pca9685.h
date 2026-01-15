#pragma once
// #include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct {
    i2c_master_dev_handle_t dev;
    uint8_t address;
} pca9685_t;

esp_err_t pca9685_init(
    pca9685_t *pca,
    i2c_master_bus_handle_t bus,
    uint8_t address
);

esp_err_t pca9685_set_pwm_freq(pca9685_t *pca, float freq_hz);
esp_err_t pca9685_set_pwm(pca9685_t *pca, uint8_t ch, uint16_t on, uint16_t off);