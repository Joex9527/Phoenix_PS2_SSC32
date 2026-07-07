#pragma once

#include <stdint.h>
#include <stdbool.h>

/*============================================================================
 *  robot_config.h — 机器人几何参数与运动学配置（纯算法配置，不依赖硬件）
 *
 *  参考: Arduino_Phoenix_Parts/Phoenix/Hex_Cfg.h
 *  硬件: hexapod-v2-7697 结构 + PCA9685 舵机
 *============================================================================*/

/* ==================== 舵机数量 ==================== */

#define NUM_LEGS            6
#define JOINTS_PER_LEG      3
#define NUM_SERVOS          (NUM_LEGS * JOINTS_PER_LEG)   // 18

/* ==================== 腿部几何参数 (mm) ==================== */

#define COXA_LENGTH         52.0f
#define FEMUR_LENGTH        82.0f
#define TIBIA_LENGTH        140.0f

/* ==================== 身体尺寸 (mm) ==================== */

#define BODY_RADIUS_X       75.0f    // 身体 X 向半径（前后）
#define BODY_RADIUS_Z       60.0f    // 身体 Z 向半径（左右）

/* ----- 各腿 Coxa 安装点相对于身体中心的偏移 (mm) ------ */
/*
 * 坐标系:
 *   X+: 前方, Y+: 上方, Z+: 右方
 *
 * 6 腿布局 (俯视图):
 *
 *        LF ──── RF
 *        /         \
 *       /           \
 *   LM |    BODY     | RM
 *       \           /
 *        \         /
 *        LR ──── RR
 *
 *  前: +X, 后: -X, 左: +Z, 右: -Z
 */

// -- 右侧 (Right) 腿: Z 偏移为负 --
#define LEG_RF_OFFSET_X     (BODY_RADIUS_X * 0.5f)     // Right Front
#define LEG_RF_OFFSET_Z    (-BODY_RADIUS_Z * 0.866f)
#define LEG_RF_OFFSET_Y     0.0f

#define LEG_RM_OFFSET_X     0.0f                       // Right Middle
#define LEG_RM_OFFSET_Z    (-BODY_RADIUS_Z)
#define LEG_RM_OFFSET_Y     0.0f

#define LEG_RR_OFFSET_X    (-BODY_RADIUS_X * 0.5f)     // Right Rear
#define LEG_RR_OFFSET_Z    (-BODY_RADIUS_Z * 0.866f)
#define LEG_RR_OFFSET_Y     0.0f

// -- 左侧 (Left) 腿: Z 偏移为正 --
#define LEG_LF_OFFSET_X     (BODY_RADIUS_X * 0.5f)     // Left Front
#define LEG_LF_OFFSET_Z     (BODY_RADIUS_Z * 0.866f)
#define LEG_LF_OFFSET_Y     0.0f

#define LEG_LM_OFFSET_X     0.0f                       // Left Middle
#define LEG_LM_OFFSET_Z     (BODY_RADIUS_Z)
#define LEG_LM_OFFSET_Y     0.0f

#define LEG_LR_OFFSET_X    (-BODY_RADIUS_X * 0.5f)     // Left Rear
#define LEG_LR_OFFSET_Z     (BODY_RADIUS_Z * 0.866f)
#define LEG_LR_OFFSET_Y     0.0f

/* ==================== 步态参数 ==================== */

#define DEFAULT_GAIT_SPEED  60       // 步态步进间隔 (ms)
#define LEG_LIFT_HEIGHT     30.0f    // 默认抬腿高度 (mm)

/* ==================== 运动限制 ==================== */

#define MAX_BODY_X          50.0f    // 身体 X 平移限制 (mm)
#define MAX_BODY_Y          50.0f    // 身体 Y 平移限制 (mm)
#define MAX_BODY_Z          50.0f    // 身体 Z 平移限制 (mm)

#define MAX_ROT_X           30.0f    // 身体 X 旋转限制 (degrees)
#define MAX_ROT_Y           30.0f    // 身体 Y 旋转限制 (degrees)
#define MAX_ROT_Z           30.0f    // 身体 Z 旋转限制 (degrees)

/* ==================== 数学宏 ==================== */

#define DEG_TO_RAD(x)       ((x) * 0.01745329252f)
#define RAD_TO_DEG(x)       ((x) * 57.29577951f)

#ifndef M_PI
#define M_PI                3.14159265358979323846f
#endif

/* ==================== 调试开关 ==================== */

#define PHOENIX_DEBUG       1

/* ==================== 控制模式与共享状态 ==================== */

/** 控制模式 (从 PS2 或上位机驱动) */
typedef enum {
    WALKMODE        = 0,
    TRANSLATEMODE   = 1,
    ROTATEMODE      = 2,
    SINGLELEGMODE   = 3,
    GPPLAYERMODE    = 4,
} ControlMode_t;

/** 摇杆死区 */
#define JOY_DEADZONE        5

/** 高度预设 */
#define DEFAULT_STAND_HEIGHT    35      /* 站立高度 (mm) */
#define DEFAULT_LEG_LIFT        50      /* 抬腿高度 (mm) */
#define HIGH_LEG_LIFT           80      /* 双倍抬腿高度 */
#define GAIT_SPEED_MIN          20      /* 最快 ms/step */
#define GAIT_SPEED_MAX          500     /* 最慢 ms/step */
#define GAIT_SPEED_STEP         50      /* 速度步长 */

/**
 * @brief 共享控制状态 (PS2 输入填充 → PhoenixLoop 消费)
 *
 * 所有控制输入（PS2 手柄、上位机串口）都将用户意图映射到此结构体。
 * PhoenixLoop 只读取此结构体，不关心输入来源。
 */
typedef struct {
    /* 行走参数 (WALKMODE) */
    int16_t  TravelX;               /* 前进/后退 速度 */
    int16_t  TravelZ;               /* 左右平移 速度 */
    int16_t  TravelY;               /* 旋转 速度 */

    /* 身体姿态偏移 */
    int16_t  BodyPosX;
    int16_t  BodyPosY;
    int16_t  BodyPosZ;
    int16_t  BodyRotX;
    int16_t  BodyRotY;
    int16_t  BodyRotZ;

    /* 模式与状态 */
    uint8_t  ControlMode;           /* ControlMode_t */
    bool     BalanceMode;
    bool     RobotOn;               /* 舵机电源 */
    bool     DoubleTravelOn;        /* 双倍步幅 */
    bool     DoubleLiftOn;          /* 双倍抬腿高度 */
    bool     WalkMethod;            /* false=M1, true=M2 */

    /* 步态与速度 */
    uint8_t  GaitType;
    int16_t  GaitSpeed;             /* ms/step */

    /* 身体姿态预设 */
    int16_t  BodyYOffset;           /* 站立: 35, 蹲下: 0 */
    int16_t  LegLiftHeight;

    /* 单腿模式 */
    uint8_t  SingleLegIndex;
    bool     SingleLegHold;
    int32_t  SingleLegX, SingleLegY, SingleLegZ;
} ControlState_t;
