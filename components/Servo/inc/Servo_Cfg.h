#pragma once

#define SERVO_COUNT 18

// PCA9685 参数
#define SERVO_PWM_FREQ 50

// 舵机物理参数（按你实物改）
#define SERVO_MIN_US   500
#define SERVO_MAX_US   2500
#define SERVO_CENTER_US 1500

// 角度范围（Phoenix 标准）
#define SERVO_ANGLE_MIN   -900
#define SERVO_ANGLE_MAX    900
