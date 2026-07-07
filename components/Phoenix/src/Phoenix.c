#include "Phoenix.h"
#include "Gait.h"
#include "IK.h"
#include "robot_config.h"
#include "platform.h"
#include "Servo.h"
#include "Servo_Cfg.h"

#include "esp_log.h"
#include <math.h>
#include <stdio.h>

static const char *TAG = "PHOENIX";

/* ---- 6 腿的 LegIK 关节角度输出 ---- */
static LegJoints_t Joints[NUM_LEGS];

/* ---- 舵机角度缓冲 (deg × 10) ---- */
static int16_t ServoAngles[NUM_SERVOS];

/* ===================================================================
 *  步态插值
 *
 *  根据当前步态类型和 Travel 参数, 计算每条腿的步态偏移。
 *
 *  轨迹形状:
 *    抬腿阶段: 正弦圆弧 (向上 + 向前)
 *    落地阶段: 直线后推 (接地)
 * =================================================================== */
static void GaitInterpolate(uint8_t leg,
                            int16_t travel_x, int16_t travel_y, int16_t travel_z,
                            int16_t lift_height,
                            float *gx, float *gy, float *gz,
                            bool *lifted)
{
    int8_t rel_step = GaitStep - GaitGetLegPhaseOffset(leg);
    if (rel_step < 0) rel_step += Gait.StepsInGait;

    uint8_t nr_lift = Gait.NrLiftedPos;
    uint8_t nr_ground = Gait.StepsInGait - nr_lift;

    *lifted = (rel_step < nr_lift);

    if (*lifted) {
        /* ----- 抬腿阶段 ----- */
        float phase = (float)rel_step / (float)nr_lift;  /* 0.0 → 1.0 */
        /* 正弦抬腿曲线 */
        *gy = sinf(phase * (float)M_PI) * (float)lift_height;
        /* 水平: 从后方移到前方 */
        float h_phase = phase - 0.5f;  /* -0.5 → +0.5 */
        *gx = h_phase * (float)travel_x;
        *gz = h_phase * (float)travel_z;
    } else {
        /* ----- 落地阶段 (推地) ----- */
        float phase = (float)(rel_step - nr_lift) / (float)nr_ground; /* 0.0 → 1.0 */
        *gy = 0.0f;
        /* 水平: 从前方向后推 */
        float h_phase = 0.5f - phase;  /* +0.5 → -0.5 */
        *gx = h_phase * (float)travel_x;
        *gz = h_phase * (float)travel_z;
    }
}

/* ===================================================================
 *  PhoenixLoop — 主控制循环
 *
 *  调用频率: 由外部 FreeRTOS 任务以 GaitSpeed 周期调用
 * =================================================================== */
void PhoenixLoop(const ControlState_t *cs)
{
    if (!cs) return;
    if (!cs->RobotOn) {
        /* 机器人关闭时保持当前位置, 不更新步态 */
        return;
    }

    /* ---- 1. 步态步进 ---- */
    GaitStepUpdate();

    /* 构建 BodyPose */
    BodyPose_t pose = {
        .BodyPosX = (float)cs->BodyPosX,
        .BodyPosY = (float)(cs->BodyPosY + cs->BodyYOffset),
        .BodyPosZ = (float)cs->BodyPosZ,
        .BodyRotX = (float)cs->BodyRotX,
        .BodyRotY = (float)cs->BodyRotY,
        .BodyRotZ = (float)cs->BodyRotZ,
    };

    /* ---- 2. 处理每条腿 ---- */
    for (uint8_t leg = 0; leg < NUM_LEGS; leg++) {

        /* 步态插值 */
        float gait_x, gait_y, gait_z;
        bool is_lifted;

        GaitInterpolate(leg,
                        cs->TravelX, cs->TravelY, cs->TravelZ,
                        cs->LegLiftHeight,
                        &gait_x, &gait_y, &gait_z,
                        &is_lifted);

        /* 单腿模式: 覆盖步态偏移 */
        if (cs->ControlMode == SINGLELEGMODE && leg == cs->SingleLegIndex) {
            gait_x += (float)cs->SingleLegX;
            gait_y += (float)cs->SingleLegY;
            gait_z += (float)cs->SingleLegZ;
        }

        /* 身体逆运动学 */
        LegTarget_t target;
        BodyIK(leg, &pose, gait_x, gait_y, gait_z, &target);
        target.lifted = is_lifted;

        /* 单腿逆运动学 */
        float coxa, femur, tibia;
        bool reachable = LegIK(target.x, target.y, target.z,
                               &coxa, &femur, &tibia);
        if (reachable) {
            Joints[leg].Coxa  = coxa;
            Joints[leg].Femur = femur;
            Joints[leg].Tibia = tibia;
        }
        /* 如果不可达, 保持上一帧的关节角度 */

        /* 舵机输出 (rad → deg×10) */
        ServoAngles[SERVO_COXA(leg)]  = (int16_t)(RAD_TO_DEG(Joints[leg].Coxa)  * 10.0f);
        ServoAngles[SERVO_FEMUR(leg)] = (int16_t)(RAD_TO_DEG(Joints[leg].Femur) * 10.0f);
        ServoAngles[SERVO_TIBIA(leg)] = (int16_t)(RAD_TO_DEG(Joints[leg].Tibia) * 10.0f);
    }

    /* ---- 3. 批量写入舵机 ---- */
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        servo_set_angle(i, ServoAngles[i]);
    }
    servo_commit();

#if PHOENIX_DEBUG
    /* 每分钟打印一次姿态信息 (~1/3600 loops at 60Hz) */
    static uint32_t debug_cnt = 0;
    if (++debug_cnt >= 3600) {
        debug_cnt = 0;
        ESP_LOGI(TAG, "Pose: Pos(%.0f,%.0f,%.0f) Rot(%.0f,%.0f,%.0f) "
                 "Travel(%d,%d,%d) Gait=%d",
                 pose.BodyPosX, pose.BodyPosY, pose.BodyPosZ,
                 pose.BodyRotX, pose.BodyRotY, pose.BodyRotZ,
                 cs->TravelX, cs->TravelY, cs->TravelZ,
                 cs->GaitType);
    }
#endif
}
