// platform.cpp
#include "platform.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/projdefs.h"

uint32_t platform_millis()
{
    return esp_timer_get_time() / 1000;
}

void platform_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
