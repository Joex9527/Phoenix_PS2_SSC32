#include "board.h"

#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"

static const char *TAG = "BOARD";

/* ---- I2C ---- */
i2c_master_bus_handle_t i2c_bus_handle = NULL;

void board_i2c_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(
        i2c_new_master_bus(&bus_cfg, &i2c_bus_handle)
    );

    ESP_LOGI(TAG, "I2C master bus initialized (SDA=%d, SCL=%d)",
             I2C_SDA_GPIO, I2C_SCL_GPIO);
}

/* ---- SPI (PS2 Controller) ---- */
spi_device_handle_t ps2_spi_handle = NULL;

void board_spi_init(void)
{
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PS2_MOSI_GPIO,
        .miso_io_num = PS2_MISO_GPIO,
        .sclk_io_num = PS2_CLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 16,          // PS2 最多 21 字节，留余量
    };

    ESP_ERROR_CHECK(
        spi_bus_initialize(PS2_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO)
    );

    spi_device_interface_config_t dev_cfg = {
        .mode = 3,                      // CPOL=1, CPHA=1
        .clock_speed_hz = 250000,       // PS2 无线: ≤500kHz, 保守用 250kHz
        .spics_io_num = PS2_CS_GPIO,    // 硬件 CS
        .queue_size = 1,
        .flags = SPI_DEVICE_BIT_LSBFIRST, // PS2 是 LSB first
    };

    ESP_ERROR_CHECK(
        spi_bus_add_device(PS2_SPI_HOST, &dev_cfg, &ps2_spi_handle)
    );

    ESP_LOGI(TAG, "SPI bus initialized for PS2 (MOSI=%d, MISO=%d, CLK=%d, CS=%d)",
             PS2_MOSI_GPIO, PS2_MISO_GPIO, PS2_CLK_GPIO, PS2_CS_GPIO);
}

/* ---- UART (上位机调试) ---- */

void board_uart_init(void)
{
    uart_config_t uart_cfg = {
        .baud_rate = DEBUG_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(
        uart_driver_install(DEBUG_UART_NUM, 256, 256, 0, NULL, 0)
    );
    ESP_ERROR_CHECK(
        uart_param_config(DEBUG_UART_NUM, &uart_cfg)
    );
    ESP_ERROR_CHECK(
        uart_set_pin(DEBUG_UART_NUM, DEBUG_UART_TX, DEBUG_UART_RX,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)
    );

    ESP_LOGI(TAG, "UART initialized (port=%d, baud=%d, TX=%d, RX=%d)",
             DEBUG_UART_NUM, DEBUG_UART_BAUD, DEBUG_UART_TX, DEBUG_UART_RX);
}

/* ---- 统一初始化 ---- */

void board_init(void)
{
    board_i2c_init();
    board_spi_init();
    board_uart_init();
    ESP_LOGI(TAG, "All board peripherals initialized");
}
