#include "board.h"
#include "Hex_Cfg.h"

#include "esp_log.h"

static const char *TAG = "BOARD_I2C";

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

    ESP_LOGI(TAG, "I2C master bus initialized");
}
