#include "Gait.h"
#include "robot_config.h"

/* ---- 全局状态 ---- */
Gait_t Gait;
int8_t GaitStep = 0;
uint8_t CurrentGaitType = GAIT_TRIPOD_6;

/* ---- 各步态的 Phase 偏移（每条腿在周期中的起始位置） ---- */
/*
 * 腿编号: 0=RR, 1=RM, 2=RF, 3=LR, 4=LM, 5=LF
 *
 * Tripod 6: 交替的 3+3 腿组
 *   Group A (step 0): RR, RF, LM
 *   Group B (step 3): RM, LR, LF
 */
static const GaitPhase_t PhaseTripod6 = {
    .LegPhase = { 0, 3, 0, 3, 0, 3 }
};

static const GaitPhase_t PhaseTripod8 = {
    .LegPhase = { 0, 4, 0, 4, 0, 4 }
};

/* Ripple: 每条腿错开 1 步 */
static const GaitPhase_t PhaseRipple6 = {
    .LegPhase = { 0, 1, 2, 3, 4, 5 }
};

static const GaitPhase_t PhaseRipple12 = {
    .LegPhase = { 0, 2, 4, 6, 8, 10 }
};

/* 当前使用的 Phase 表 */
static const GaitPhase_t *CurrentPhase = &PhaseTripod6;

/* =================================================================== */

void GaitSelect(uint8_t gaitType)
{
    CurrentGaitType = gaitType;

    switch (gaitType) {
    case GAIT_TRIPOD_6:
    default:
        Gait.StepsInGait  = 6;
        Gait.NrLiftedPos  = 3;
        Gait.FrontDownPos = 2;
        Gait.LiftDivFactor = 2;
        CurrentPhase = &PhaseTripod6;
        break;

    case GAIT_TRIPOD_8:
        Gait.StepsInGait  = 8;
        Gait.NrLiftedPos  = 3;
        Gait.FrontDownPos = 3;
        Gait.LiftDivFactor = 2;
        CurrentPhase = &PhaseTripod8;
        break;

    case GAIT_RIPPLE_6:
        Gait.StepsInGait  = 6;
        Gait.NrLiftedPos  = 2;
        Gait.FrontDownPos = 2;
        Gait.LiftDivFactor = 2;
        CurrentPhase = &PhaseRipple6;
        break;

    case GAIT_RIPPLE_12:
        Gait.StepsInGait  = 12;
        Gait.NrLiftedPos  = 3;
        Gait.FrontDownPos = 4;
        Gait.LiftDivFactor = 2;
        CurrentPhase = &PhaseRipple12;
        break;
    }

    GaitStep = 0;
}

void GaitStepUpdate(void)
{
    GaitStep++;
    if (GaitStep >= Gait.StepsInGait)
        GaitStep = 0;
}

bool GaitIsLegLifted(uint8_t leg)
{
    if (leg >= NUM_LEGS) return false;

    int8_t phase = CurrentPhase->LegPhase[leg];
    int8_t rel_step = GaitStep - phase;
    if (rel_step < 0) rel_step += Gait.StepsInGait;

    return (rel_step < Gait.NrLiftedPos);
}

float GaitGetLegPhase(uint8_t leg)
{
    if (leg >= NUM_LEGS) return 0.0f;

    int8_t phase = CurrentPhase->LegPhase[leg];
    int8_t rel_step = GaitStep - phase;
    if (rel_step < 0) rel_step += Gait.StepsInGait;

    return (float)rel_step / (float)Gait.StepsInGait;
}

int8_t GaitGetLegPhaseOffset(uint8_t leg)
{
    if (leg >= NUM_LEGS) return 0;
    return CurrentPhase->LegPhase[leg];
}
