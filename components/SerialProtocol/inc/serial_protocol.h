#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ===================================================================
 *  Hexapod 上位机串口协议
 *
 *  协议: LewanSoul/Lobot 0xFE 0xFE 帧格式 (扩展为 18 舵机)
 *  物理层: USB Serial/JTAG, 115200-8N1
 *
 *  帧格式:
 *    Byte 0:     0xFE          Header 1
 *    Byte 1:     0xFE          Header 2
 *    Byte 2:     Length        从本字节到 0xFA 之间的字节数
 *    Byte 3:     Command       命令码
 *    Byte 4..N-1: Payload      可变长度
 *    Byte N:     0xFA          End marker
 *
 *  角度编码: angle_deg * 100 → int16 (高位在前, 有符号)
 * =================================================================== */

/* ---- 命令码 ---- */
#define SP_CMD_POWER_ON         0x10    /* 上电/初始化 */
#define SP_CMD_READ_ANGLES      0x20    /* 读取所有角度 */
#define SP_CMD_SET_SINGLE       0x21    /* 设置单个舵机角度 */
#define SP_CMD_SET_ALL          0x22    /* 设置 18 舵机角度 */
#define SP_CMD_TORQUE_OFF       0x56    /* 关闭单个舵机扭矩 */
#define SP_CMD_TORQUE_ON        0x57    /* 开启单个舵机扭矩 */

/* ---- 协议常量 ---- */
#define SP_HEADER1              0xFE
#define SP_HEADER2              0xFE
#define SP_END                  0xFA
#define SP_MAX_PAYLOAD          40      /* 18 舵机 × 2 字节 + 1 速度 = 37 */
#define SP_RX_BUF_SIZE          64
#define SP_TX_BUF_SIZE          64

/* ---- 解析器状态 ---- */
typedef enum {
    SP_IDLE = 0,
    SP_GOT_FE1,         /* 收到第一个 0xFE */
    SP_GOT_FE2,         /* 收到第二个 0xFE */
    SP_GOT_LENGTH,      /* 收到长度 */
    SP_IN_PAYLOAD,      /* 正在接收负载 */
} SP_State_t;

/* ---- 协议上下文 ---- */
typedef struct {
    SP_State_t  state;
    uint8_t     length;                     /* 负载字节数 */
    uint8_t     cmd;
    uint8_t     payload[SP_MAX_PAYLOAD];
    uint8_t     payload_idx;
    bool        has_control;                /* 上位机是否正在控制 (优先级高于 PS2) */
    uint32_t    last_cmd_time;             /* 最后收到命令的时间 (用于超时检测) */
} SerialProto_t;

/* ---- 命令处理回调 ---- */
typedef struct {
    /** 已解析的角度数组 (单位: 度×100, 18 个舵机) */
    int16_t angles_deg100[18];
    /** 是否有新数据 */
    bool    new_data;
    /** 是否启用舵机 */
    bool    servos_enabled;
} SerialInput_t;

/* ==================== API ==================== */

/**
 * @brief 初始化串口协议解析器
 * @param proto    协议上下文
 * @param uart_num UART 端口号
 */
void serial_proto_init(SerialProto_t *proto, int uart_num);

/**
 * @brief 从 UART 读取并解析帧
 * @param proto 协议上下文
 * @param input 输出: 解析到的舵机角度和控制命令
 *
 * 每个主循环周期调用一次。
 * 非阻塞 — 如果没有可用数据, 立即返回。
 */
void serial_proto_poll(SerialProto_t *proto, SerialInput_t *input);

/**
 * @brief 发送角度回读响应 (命令 0x20)
 * @param angles 18 个舵机的当前角度 (度×10, 即实际角度角)
 */
void serial_proto_send_angles(const int16_t *angles);

/**
 * @brief 检查上位机是否接管控制权
 */
bool serial_proto_has_control(const SerialProto_t *proto);
