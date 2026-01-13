#pragma once
#include <stdint.h>

struct Gait_t {
    uint8_t  StepsInGait;
    uint8_t  NrLiftedPos;
    uint8_t  FrontDownPos;
    uint8_t  LiftDivFactor;
};

extern Gait_t Gait;
extern int8_t GaitStep;

void GaitSelect(uint8_t gaitType);
void GaitStepUpdate();
