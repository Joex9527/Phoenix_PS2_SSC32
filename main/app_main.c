#include "platform.h"
#include "Gait.h"
// #include "Phoenix.h"

#include "board.h"
#include "pca9685.h"

static pca9685_t pca;

void app_main(void)
{
    board_i2c_init();

    pca9685_init(&pca, i2c_bus_handle, 0x40);
    pca9685_set_pwm_freq(&pca, 50.0f);

    while (1) {
        pca9685_set_pwm(&pca, 0, 0, 307); // ~1.5ms
        platform_delay(1000u);
    }
}

#if 0
void app_main(void)
{
    GaitSelect(0);

    while (1)
    {
        PhoenixLoop();
        platform_delay(5);
    }
}
#endif