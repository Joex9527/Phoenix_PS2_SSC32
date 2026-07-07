#pragma once
#include <stdint.h>
#include "pca9685.h"

/**
 * @brief 初始化舵机子系统
 * @param pca 已初始化的 PCA9685 设备句柄
 */
void servo_init(pca9685_t *pca);

/**
 * @brief 设置单个舵机的目标角度
 * @param index 舵机索引 (0 ~ SERVO_COUNT-1)
 * @param angle 目标角度 (单位: 0.1 度, 范围 -900 ~ +900)
 *
 * 角度不会立即生效，需调用 servo_commit() 批量更新。
 */
void servo_set_angle(uint8_t index, int16_t angle);

/**
 * @brief 批量提交所有舵机角度到 PCA9685
 *
 * 将所有缓冲的角度一次性写入 PCA9685 的 18 个通道。
 */
void servo_commit(void);

/**
 * @brief 释放所有舵机（关闭 PWM 输出）
 */
void servo_release(void);
