#pragma once
#include <stdbool.h>
#include <stdint.h>

/* ==================== 坐标与角度类型 ==================== */

/** 三维坐标 (足端位置, mm) */
typedef struct {
    float x;        /* 前 (+) / 后 (-) */
    float y;        /* 上 (+) / 下 (-) */
    float z;        /* 右 (+) / 左 (-) */
} Point3D_t;

/** 单腿关节角度 (弧度) */
typedef struct {
    float Coxa;     /* 基节 (Hip) — 水平旋转 */
    float Femur;    /* 股节 (Knee) — 垂直旋转 */
    float Tibia;    /* 胫节 (Ankle) — 垂直旋转 */
} LegJoints_t;

/** 身体姿态 */
typedef struct {
    float BodyPosX, BodyPosY, BodyPosZ;   /* 身体平移 (mm) */
    float BodyRotX, BodyRotY, BodyRotZ;   /* 身体旋转 (deg) */
} BodyPose_t;

/** 单腿足端目标 */
typedef struct {
    float x, y, z;  /* 足端在腿局部坐标系的坐标 (mm) */
    bool  lifted;   /* 当前是否抬腿 */
} LegTarget_t;

/* ==================== API ==================== */

/**
 * @brief 单腿逆运动学：足端坐标 → 关节角度
 * @param[in]  x, y, z  足端在腿局部坐标系的目标位置 (mm)
 * @param[out] coxa     基节角 (rad)
 * @param[out] femur    股节角 (rad)
 * @param[out] tibia    胫节角 (rad)
 * @return true 可到达, false 超出工作空间
 */
bool LegIK(float x, float y, float z,
           float *coxa,
           float *femur,
           float *tibia);

/**
 * @brief 身体逆运动学：身体姿态 → 单腿足端目标坐标
 * @param[in]  leg      腿编号 (0=RR, 1=RM, 2=RF, 3=LR, 4=LM, 5=LF)
 * @param[in]  pose     身体姿态
 * @param[in]  gait_x   步态插值 X (行走方向)
 * @param[in]  gait_y   步态插值 Y (抬腿高度)
 * @param[in]  gait_z   步态插值 Z (侧移)
 * @param[out] target   该腿的足端目标坐标
 */
void BodyIK(uint8_t leg,
            const BodyPose_t *pose,
            float gait_x, float gait_y, float gait_z,
            LegTarget_t *target);
