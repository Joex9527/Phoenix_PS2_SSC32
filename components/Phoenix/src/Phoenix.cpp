#include "platform.h"
#include "Hex_Cfg.h"

#include "Phoenix.h"
#include "Gait.h"
#include "IK.h"

#include <stdio.h>

void PhoenixLoop()
{
    static uint32_t last = 0;
    uint32_t now = platform_millis();

    if (now - last < DEFAULT_GAIT_SPEED)
        return;

    last = now;
    GaitStepUpdate();

    for (int leg = 0; leg < 6; leg++)
    {
        float x = 0;
        float y = (GaitStep == leg % 2) ? -LEG_LIFT_HEIGHT : 0;
        float z = 100;

        float coxa, femur, tibia;
        LegIK(x, y, z, coxa, femur, tibia);

#if PHOENIX_DEBUG
        printf("Leg %d: C=%.1f F=%.1f T=%.1f\n",
               leg,
               RAD_TO_DEG(coxa),
               RAD_TO_DEG(femur),
               RAD_TO_DEG(tibia));
#endif
    }
}
