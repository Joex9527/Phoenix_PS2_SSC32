#pragma once
#include "robot_config.h"   /* for ControlState_t */

/**
 * @brief Phoenix 六足机器人主控制循环
 * @param cs 当前控制状态（由 PS2 或上位机填充）
 *
 * 流程:
 *   1. 步态引擎 — 更新 GaitStep, 确定各腿抬腿/落地状态
 *   2. BodyIK   — 身体姿态 → 6 腿足端目标坐标
 *   3. LegIK    — 足端坐标 → 各腿关节角度
 *   4. Servo    — 关节角度 → PWM 输出
 *
 * 调用频率: 50-100Hz (由调用者控制)
 */
void PhoenixLoop(const ControlState_t *cs);
