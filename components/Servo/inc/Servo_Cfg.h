#pragma once
#include "robot_config.h"   /* NUM_LEGS, JOINTS_PER_LEG, NUM_SERVOS */

/*============================================================================
 *  Servo_Cfg.h — 舵机物理参数与通道映射
 *
 *  18 个舵机: 6 腿 × 3 关节 (Coxa, Femur, Tibia)
 *  PCA9685 通道映射: 腿编号 0-5 → 通道 0-17
 *============================================================================*/

#define SERVO_COUNT          NUM_SERVOS      /* = NUM_LEGS * JOINTS_PER_LEG = 18 */

/* ---- PCA9685 参数 ---- */
#define SERVO_PWM_FREQ       50        // Hz

/* ---- 舵机脉宽范围 (按实际舵机标定) ---- */
#define SERVO_MIN_US         500       // 最小脉宽 (us)
#define SERVO_MAX_US         2500      // 最大脉宽 (us)
#define SERVO_CENTER_US      1500      // 中立点脉宽 (us)

/* ---- 角度范围 (Phoenix 标准: 1/10 度) ---- */
#define SERVO_ANGLE_MIN      -900      // -90.0 度
#define SERVO_ANGLE_MAX       900      // +90.0 度

/* ==================== 舵机 → PCA9685 通道映射 ==================== */

/*
 * 腿编号 (与 Phoenix BodyIK 一致):
 *   0 = Right Rear   (RR)
 *   1 = Right Middle (RM)
 *   2 = Right Front  (RF)
 *   3 = Left Rear    (LR)
 *   4 = Left Middle  (LM)
 *   5 = Left Front   (LF)
 *
 * 每腿 3 个关节:
 *   Joint 0 = Coxa  (基节/Hip)
 *   Joint 1 = Femur (股节/Knee)
 *   Joint 2 = Tibia (胫节/Ankle)
 *
 * PCA9685 通道 = leg * 3 + joint
 */

/* ---- 便捷索引宏 ---- */
#define SERVO_IDX(leg, joint)   ((leg) * JOINTS_PER_LEG + (joint))
#define SERVO_COXA(leg)         SERVO_IDX(leg, 0)
#define SERVO_FEMUR(leg)        SERVO_IDX(leg, 1)
#define SERVO_TIBIA(leg)        SERVO_IDX(leg, 2)

/* ---- 舵机方向校正 (+1 或 -1, 用于补偿机械安装方向) ---- */
#define LEG_RR_COXA_DIR     +1
#define LEG_RR_FEMUR_DIR    +1
#define LEG_RR_TIBIA_DIR    +1
#define LEG_RM_COXA_DIR     +1
#define LEG_RM_FEMUR_DIR    +1
#define LEG_RM_TIBIA_DIR    +1
#define LEG_RF_COXA_DIR     +1
#define LEG_RF_FEMUR_DIR    +1
#define LEG_RF_TIBIA_DIR    +1
#define LEG_LR_COXA_DIR     -1     /* 左腿通常是镜面安装, 需反转 */
#define LEG_LR_FEMUR_DIR    -1
#define LEG_LR_TIBIA_DIR    -1
#define LEG_LM_COXA_DIR     -1
#define LEG_LM_FEMUR_DIR    -1
#define LEG_LM_TIBIA_DIR    -1
#define LEG_LF_COXA_DIR     -1
#define LEG_LF_FEMUR_DIR    -1
#define LEG_LF_TIBIA_DIR    -1
