#include "IK.h"
#include "Hex_Cfg.h"
#include <math.h>

bool LegIK(float x, float y, float z,
           float& coxa,
           float& femur,
           float& tibia)
{
    coxa = atan2f(x, z);

    float L = sqrtf(x*x + z*z) - COXA_LENGTH;
    float D = sqrtf(L*L + y*y);

    float a1 = atan2f(y, L);
    float a2 = acosf(
        (FEMUR_LENGTH*FEMUR_LENGTH + D*D - TIBIA_LENGTH*TIBIA_LENGTH) /
        (2 * FEMUR_LENGTH * D)
    );

    femur = a1 + a2;
    tibia = acosf(
        (FEMUR_LENGTH*FEMUR_LENGTH + TIBIA_LENGTH*TIBIA_LENGTH - D*D) /
        (2 * FEMUR_LENGTH * TIBIA_LENGTH)
    ) - M_PI;

    return true;
}
