// Gait.cpp
#include "Gait.h"

Gait_t Gait;
int8_t GaitStep = 0;

void GaitSelect(uint8_t gaitType)
{
    // Tripod
    Gait.StepsInGait  = 6;
    Gait.NrLiftedPos  = 3;
    Gait.FrontDownPos = 2;
    Gait.LiftDivFactor = 2;
}

void GaitStepUpdate()
{
    GaitStep++;
    if (GaitStep >= Gait.StepsInGait)
        GaitStep = 0;
}
