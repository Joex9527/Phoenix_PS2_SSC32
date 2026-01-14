#include "platform.h"
#include "Gait.h"
#include "Phoenix.h"

extern "C" void app_main(void)
{
    GaitSelect(0);

    while (1)
    {
        PhoenixLoop();
        platform_delay(5);
    }
}
