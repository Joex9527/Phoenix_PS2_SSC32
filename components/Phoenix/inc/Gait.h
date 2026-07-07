#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ==================== 步态类型定义 ==================== */

/** 步态参数 */
typedef struct {
    uint8_t  StepsInGait;       /* 一个步态周期的总步数 */
    uint8_t  NrLiftedPos;       /* 每条腿在周期中抬起的位置数 */
    uint8_t  FrontDownPos;      /* 前腿放下位置 (步数偏移) */
    uint8_t  LiftDivFactor;     /* 抬腿分割因子 */
} Gait_t;

/** 步态类型枚举 */
typedef enum {
    GAIT_TRIPOD_6 = 0,          /* 6 步三脚步态（默认） */
    GAIT_TRIPOD_8,              /* 8 步三脚步态 */
    GAIT_RIPPLE_6,              /* 6 步波形步态 */
    GAIT_RIPPLE_12,             /* 12 步波形步态 */
    NUM_GAITS
} GaitType_t;

/** 每条腿在步态周期中的相位偏移 */
typedef struct {
    int8_t LegPhase[6];         /* 0..StepsInGait-1 */
} GaitPhase_t;

/* ==================== 全局状态 ==================== */

extern Gait_t Gait;
extern int8_t GaitStep;         /* 当前步态步数 (0..StepsInGait-1) */
extern uint8_t CurrentGaitType;

/* ==================== API ==================== */

/**
 * @brief 选择并初始化步态
 * @param gaitType 步态类型 (GaitType_t)
 */
void GaitSelect(uint8_t gaitType);

/**
 * @brief 步态步进（每个主循环周期调用一次）
 *        递增 GaitStep，超出范围则回绕
 */
void GaitStepUpdate(void);

/**
 * @brief 查询指定腿当前是否处于抬腿阶段
 * @param  leg 腿编号 (0-5)
 * @return true=抬腿, false=落地
 */
bool GaitIsLegLifted(uint8_t leg);

/**
 * @brief 获取指定腿在步态周期中的归一化进度 (0.0 ~ 1.0)
 */
float GaitGetLegPhase(uint8_t leg);

/**
 * @brief 获取指定腿在步态周期中的相位偏移 (原始整数值)
 * @param leg 腿编号 (0-5)
 * @return 相位偏移 (0..StepsInGait-1)
 */
int8_t GaitGetLegPhaseOffset(uint8_t leg);
