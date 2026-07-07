#include "pca9685.h"
#include "platform.h"

#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "PCA9685";

static esp_err_t write_reg(pca9685_t *pca, uint8_t reg, uint8_t val)
{
    uint8_t data[2] = { reg, val };
    return i2c_master_transmit(pca->dev, data, sizeof(data), -1);
}

esp_err_t pca9685_init(pca9685_t *pca, i2c_master_bus_handle_t bus, uint8_t address)
{
    pca->address = address;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = PCA9685_I2C_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &pca->dev));

    /* Reset MODE1 */
    ESP_ERROR_CHECK(write_reg(pca, PCA9685_MODE1, 0x00));
    platform_delay(10u);

    ESP_LOGI(TAG, "PCA9685 initialized at address 0x%02X", address);
    return ESP_OK;
}

esp_err_t pca9685_set_pwm_freq(pca9685_t *pca, float freq_hz)
{
    float prescale_f = 25000000.0f / (4096.0f * freq_hz) - 1.0f;
    uint8_t prescale = (uint8_t)(prescale_f + 0.5f);

    /* Enter sleep mode to set prescaler */
    uint8_t mode1 = 0x10;   /* SLEEP bit */
    ESP_ERROR_CHECK(write_reg(pca, PCA9685_MODE1, mode1));
    ESP_ERROR_CHECK(write_reg(pca, PCA9685_PRESCALE, prescale));
    /* Wake up with auto-increment enabled */
    ESP_ERROR_CHECK(write_reg(pca, PCA9685_MODE1, 0xA1));

    platform_delay(5u);

    ESP_LOGI(TAG, "PWM frequency set to %.1f Hz (prescaler=%d)", freq_hz, prescale);
    return ESP_OK;
}

esp_err_t pca9685_set_pwm(pca9685_t *pca, uint8_t ch, uint16_t on, uint16_t off)
{
    uint8_t reg = LED0_ON_L + 4 * ch;
    uint8_t data[5] = {
        reg,
        on & 0xFF, on >> 8,
        off & 0xFF, off >> 8
    };

    return i2c_master_transmit(
        pca->dev, data, sizeof(data), -1
    );
}

esp_err_t pca9685_sleep(pca9685_t *pca)
{
    /* SLEEP=1, disable internal oscillator */
    return write_reg(pca, PCA9685_MODE1, 0x10);
}

esp_err_t pca9685_wake(pca9685_t *pca)
{
    /* SLEEP=0, auto-increment on, restart */
    return write_reg(pca, PCA9685_MODE1, 0xA1);
}
