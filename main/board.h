#pragma once

#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "driver/uart.h"

/*============================================================================
 *  board.h — 硬件引脚映射与板级初始化
 *
 *  目标平台: ESP32-S3
 *  舵机驱动: PCA9685 (I2C)
 *  手柄输入: PS2 Wireless (SPI)
 *  调试接口: USB Serial/JTAG (UART)
 *============================================================================*/

/* ==================== I2C (PCA9685) ==================== */

#define I2C_PORT_NUM        0
#define I2C_SDA_GPIO        8
#define I2C_SCL_GPIO        9
#define I2C_FREQ_HZ         400000      // 400kHz

extern i2c_master_bus_handle_t i2c_bus_handle;

/* ==================== SPI (PS2 Controller) ==================== */

#define PS2_SPI_HOST        SPI2_HOST
#define PS2_MOSI_GPIO       11          // CMD
#define PS2_MISO_GPIO       13          // DAT (需外部 4.7k 上拉到 3.3V)
#define PS2_CLK_GPIO        12          // CLK
#define PS2_CS_GPIO         10          // ATT (Chip Select, active LOW)

extern spi_device_handle_t ps2_spi_handle;

/* ==================== UART (上位机调试) ==================== */

#define DEBUG_UART_NUM      UART_NUM_0
#define DEBUG_UART_BAUD     115200
#define DEBUG_UART_TX       43          // ESP32-S3 USB Serial JTAG → GPIO43
#define DEBUG_UART_RX       44          // ESP32-S3 USB Serial JTAG → GPIO44

/* ==================== 板级初始化 ==================== */

void board_i2c_init(void);
void board_spi_init(void);
void board_uart_init(void);
void board_init(void);                  // 一次性初始化全部外设
