#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"   /* for spi_device_handle_t */

/* ===================================================================
 *  PS2 手柄底层 SPI 驱动
 *
 *  协议: PS2 Wireless (SPI Mode 3, LSB first, 250kHz)
 *  参考: madsci1016/Arduino-PS2X, Bill Porter's PS2X library
 * =================================================================== */

/* ---- PS2 按钮位掩码 (16-bit) ---- */
#define PSB_SELECT      0x0001
#define PSB_L3          0x0002
#define PSB_R3          0x0004
#define PSB_START       0x0008
#define PSB_PAD_UP      0x0010
#define PSB_PAD_RIGHT   0x0020
#define PSB_PAD_DOWN    0x0040
#define PSB_PAD_LEFT    0x0080
#define PSB_L2          0x0100
#define PSB_R2          0x0200
#define PSB_L1          0x0400
#define PSB_R1          0x0800
#define PSB_TRIANGLE    0x1000
#define PSB_CIRCLE      0x2000
#define PSB_CROSS       0x4000
#define PSB_SQUARE      0x8000

/* 便捷别名 */
#define PSB_GREEN       PSB_TRIANGLE
#define PSB_RED         PSB_CIRCLE
#define PSB_BLUE        PSB_CROSS
#define PSB_PINK        PSB_SQUARE

/* ---- 摇杆轴索引 ---- */
#define PSS_RX          5       /* 右摇杆 X: 0=左, 128=中, 255=右 */
#define PSS_RY          6       /* 右摇杆 Y: 0=上, 128=中, 255=下 */
#define PSS_LX          7       /* 左摇杆 X: 0=左, 128=中, 255=右 */
#define PSS_LY          8       /* 左摇杆 Y: 0=上, 128=中, 255=下 */

/* ---- PS2 手柄完整状态 ---- */
typedef struct {
    uint16_t buttons;           /* 16-bit 按钮位掩码 */
    uint16_t prev_buttons;      /* 上一帧的按钮状态 (用于边沿检测) */
    uint8_t  axes[9];           /* 摇杆轴值 [0..255] (索引 5-8 有效) */
    bool     connected;         /* 手柄是否已连接 */
    bool     analog_mode;       /* 是否处于模拟模式 */
} PS2State_t;

/* ==================== API ==================== */

/**
 * @brief 初始化 PS2 手柄并进入模拟模式
 * @param spi_handle 已初始化的 SPI 设备句柄 (由 board_spi_init 创建)
 * @return true=成功, false=未检测到手柄
 */
bool ps2_init(spi_device_handle_t spi_handle);

/**
 * @brief 轮询一次手柄状态 (发送 9-byte 命令, 读取按钮+摇杆)
 *         调用频率: 50-100 Hz
 * @param state 输出手柄状态
 * @return true=数据有效, false=通信错误
 */
bool ps2_poll(PS2State_t *state);

/**
 * @brief 检查按钮是否按下 (持续检测, 按住期间一直返回 true)
 */
bool ps2_button(uint16_t buttons, uint16_t mask);

/**
 * @brief 检查按钮是否刚被按下 (边沿检测, 只在按下瞬间返回一次 true)
 */
bool ps2_button_pressed(const PS2State_t *state, uint16_t mask);

/**
 * @brief 检查按钮是否刚被释放
 */
bool ps2_button_released(const PS2State_t *state, uint16_t mask);

/**
 * @brief 检查是否有任何按钮状态变化
 */
bool ps2_new_button_state(const PS2State_t *state);

/**
 * @brief 读取摇杆轴值 (0-255, 128=中心)
 */
uint8_t ps2_analog(const PS2State_t *state, uint8_t axis);
