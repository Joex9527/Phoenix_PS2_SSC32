#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "robot_config.h"       /* ControlMode_t, ControlState_t, JOY_DEADZONE */

/* ===================================================================
 *  PS2 → 控制状态映射
 *
 *  5 种控制模式:
 *    WALKMODE      (默认) — 行走 + 旋转
 *    TRANSLATEMODE (L1)   — 身体平移
 *    ROTATEMODE    (L2)   — 身体旋转
 *    SINGLELEGMODE (○)    — 单腿控制
 *    GPPLAYERMODE         — 预留, 动作序列回放
 * =================================================================== */

/* ---- 默认控制状态 ---- */
/* 常量已移至 robot_config.h: DEFAULT_STAND_HEIGHT, DEFAULT_LEG_LIFT, HIGH_LEG_LIFT, etc. */

/* ==================== API ==================== */

/**
 * @brief 初始化控制状态为默认值
 */
void control_state_init(ControlState_t *cs);

/**
 * @brief 从 PS2 手柄状态更新控制状态
 * @param cs     控制状态 (读写)
 * @param ps2    手柄状态 PS2State_t (只读, 通过 void* 避免循环依赖)
 *
 * 此函数处理:
 *   - 模式切换 (START, L1, L2, CIRCLE, R3)
 *   - 摇杆 → 运动参数映射 (取决于当前模式)
 *   - 全局按钮 (TRIANGLE, SQUARE, SELECT, R1, R2, D-Pad)
 */
void ps2_update_control(ControlState_t *cs, const void *ps2_state);
