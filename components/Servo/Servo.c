#include "Servo.h"
#include "Servo_Cfg.h"
#include "pca9685.h"
#include "Hex_Cfg.h"

static uint16_t servo_ticks[SERVO_COUNT];

static inline uint16_t us_to_ticks(uint16_t us)
{
    // 50Hz → 20ms → 20000us
    return (uint16_t)((us * 4096UL) / 20000UL);
}

/**
 * Convert angle to pulse width.
 */
static inline uint16_t angle_to_us(int16_t angle)
{
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    int32_t us =
        SERVO_CENTER_US +
        (int32_t)angle * (SERVO_MAX_US - SERVO_MIN_US) / 1800;

    return (uint16_t)us;
}

void servo_init()
{
    pca9685_init(I2C_NUM_0, 0x40, 400000);
    pca9685_set_pwm_freq(50.0f);
}

void servo_set_angle(uint8_t index, int16_t angle)
{
    if (index >= SERVO_COUNT) return;

    uint16_t us = angle_to_us(angle);
    servo_ticks[index] = us_to_ticks(us);
}

void servo_commit()
{
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        pca9685_set_pwm(i, 0, servo_ticks[i]);
    }
}