#include "ps2_control.h"
#include "ps2_driver.h"
#include "robot_config.h"
#include "Gait.h"

#include "esp_log.h"

static const char *TAG = "PS2CTL";

/* ---- 内部辅助 ---- */

/** 摇杆值 → 带死区的偏移量 (范围: -128 ~ +127) */
static inline int16_t joy_delta(uint8_t raw)
{
    int16_t d = (int16_t)raw - 128;
    if (d > -JOY_DEADZONE && d < JOY_DEADZONE) return 0;
    return d;
}

/* =================================================================== */

void control_state_init(ControlState_t *cs)
{
    cs->TravelX       = 0;
    cs->TravelY       = 0;
    cs->TravelZ       = 0;
    cs->BodyPosX      = 0;
    cs->BodyPosY      = 0;
    cs->BodyPosZ      = 0;
    cs->BodyRotX      = 0;
    cs->BodyRotY      = 0;
    cs->BodyRotZ      = 0;
    cs->ControlMode   = WALKMODE;
    cs->BalanceMode   = false;
    cs->RobotOn       = false;          /* 默认关闭, 按 START 启动 */
    cs->DoubleTravelOn = false;
    cs->DoubleLiftOn  = false;
    cs->WalkMethod    = false;
    cs->GaitType      = GAIT_TRIPOD_6;
    cs->GaitSpeed     = DEFAULT_GAIT_SPEED;
    cs->BodyYOffset   = 0;             /* 蹲下状态 */
    cs->LegLiftHeight = DEFAULT_LEG_LIFT;
    cs->SingleLegIndex = 0;
    cs->SingleLegHold = false;
    cs->SingleLegX    = 0;
    cs->SingleLegY    = 0;
    cs->SingleLegZ    = 0;
}

void ps2_update_control(ControlState_t *cs, const void *ps2_ptr)
{
    const PS2State_t *ps2 = (const PS2State_t *)ps2_ptr;
    if (!ps2 || !ps2->connected) return;

    uint16_t btns = ps2->buttons;
    uint8_t  lx = ps2_analog(ps2, PSS_LX);
    uint8_t  ly = ps2_analog(ps2, PSS_LY);
    uint8_t  rx = ps2_analog(ps2, PSS_RX);
    uint8_t  ry = ps2_analog(ps2, PSS_RY);

    /* ================================================================
     *  全局控制 (所有模式通用)
     * ================================================================ */

    /* START: 切换舵机电源 */
    if (ps2_button_pressed(ps2, PSB_START)) {
        cs->RobotOn = !cs->RobotOn;
        if (cs->RobotOn) {
            cs->BodyYOffset = DEFAULT_STAND_HEIGHT;
            ESP_LOGI(TAG, "Robot ON, standing up");
        } else {
            cs->BodyYOffset = 0;
            ESP_LOGI(TAG, "Robot OFF, sitting down");
        }
    }

    /* TRIANGLE: 站立/蹲下 */
    if (ps2_button_pressed(ps2, PSB_TRIANGLE)) {
        if (cs->BodyYOffset > 0) {
            cs->BodyYOffset = 0;
        } else {
            cs->BodyYOffset = DEFAULT_STAND_HEIGHT;
        }
        ESP_LOGI(TAG, "Body Y offset = %d mm", cs->BodyYOffset);
    }

    /* SQUARE: 平衡模式 */
    if (ps2_button_pressed(ps2, PSB_SQUARE)) {
        cs->BalanceMode = !cs->BalanceMode;
        ESP_LOGI(TAG, "Balance mode: %s", cs->BalanceMode ? "ON" : "OFF");
    }

    /* R1: 双倍抬腿高度 */
    if (ps2_button_pressed(ps2, PSB_R1)) {
        cs->DoubleLiftOn = !cs->DoubleLiftOn;
        cs->LegLiftHeight = cs->DoubleLiftOn ? HIGH_LEG_LIFT : DEFAULT_LEG_LIFT;
        ESP_LOGI(TAG, "Leg lift height: %d mm", cs->LegLiftHeight);
    }

    /* D-Pad UP: 身体升高 */
    if (ps2_button_pressed(ps2, PSB_PAD_UP)) {
        cs->BodyPosY += 10;
        if (cs->BodyPosY > MAX_BODY_Y) cs->BodyPosY = MAX_BODY_Y;
    }
    /* D-Pad DOWN: 身体降低 */
    if (ps2_button_pressed(ps2, PSB_PAD_DOWN)) {
        cs->BodyPosY -= 10;
        if (cs->BodyPosY < -MAX_BODY_Y) cs->BodyPosY = -MAX_BODY_Y;
    }
    /* D-Pad LEFT: 减速 */
    if (ps2_button_pressed(ps2, PSB_PAD_LEFT)) {
        cs->GaitSpeed += GAIT_SPEED_STEP;
        if (cs->GaitSpeed > GAIT_SPEED_MAX) cs->GaitSpeed = GAIT_SPEED_MAX;
    }
    /* D-Pad RIGHT: 加速 */
    if (ps2_button_pressed(ps2, PSB_PAD_RIGHT)) {
        cs->GaitSpeed -= GAIT_SPEED_STEP;
        if (cs->GaitSpeed < GAIT_SPEED_MIN) cs->GaitSpeed = GAIT_SPEED_MIN;
    }

    /* ================================================================
     *  模式切换
     * ================================================================ */

    /* L1: 切换身体平移模式 */
    if (ps2_button(btns, PSB_L1)) {
        if (cs->ControlMode != TRANSLATEMODE) {
            cs->ControlMode = TRANSLATEMODE;
        }
    }
    /* L2: 切换身体旋转模式 */
    else if (ps2_button(btns, PSB_L2)) {
        if (cs->ControlMode != ROTATEMODE) {
            cs->ControlMode = ROTATEMODE;
        }
    }
    /* CIRCLE: 切换单腿模式 / 返回行走 */
    else if (ps2_button_pressed(ps2, PSB_CIRCLE)) {
        if (cs->ControlMode == SINGLELEGMODE) {
            cs->ControlMode = WALKMODE;
            ESP_LOGI(TAG, "Exit single leg mode");
        } else {
            cs->ControlMode = SINGLELEGMODE;
            cs->SingleLegIndex = 0;
            cs->SingleLegX = 0;
            cs->SingleLegY = 0;
            cs->SingleLegZ = 0;
            ESP_LOGI(TAG, "Single leg mode (leg 0)");
        }
    }
    /* 无模式按钮 → 返回行走 */
    else {
        if (cs->ControlMode == TRANSLATEMODE || cs->ControlMode == ROTATEMODE) {
            cs->ControlMode = WALKMODE;
        }
    }

    /* R3 (按下右摇杆): 切换行走方式 */
    if (ps2_button_pressed(ps2, PSB_R3)) {
        cs->WalkMethod = !cs->WalkMethod;
        ESP_LOGI(TAG, "Walk method: %s", cs->WalkMethod ? "2" : "1");
    }

    /* ================================================================
     *  模式相关处理
     * ================================================================ */

    switch (cs->ControlMode) {

    case WALKMODE:
    {
        /* R2: 双倍步幅 */
        cs->DoubleTravelOn = ps2_button(btns, PSB_R2);

        int16_t div = cs->DoubleTravelOn ? 1 : 2;

        if (cs->WalkMethod == false) {
            /* Walk Method 1: 左摇杆行走, 右摇杆旋转 */
            cs->TravelX =  (joy_delta(ly)) / div;       /* 前进/后退 */
            cs->TravelZ = -(joy_delta(lx)) / div;       /* 左右平移 (反转) */
            cs->TravelY = -(joy_delta(rx)) / 4;         /* 旋转 */
        } else {
            /* Walk Method 2: 只用右摇杆 */
            cs->TravelX =  (joy_delta(ry)) / div;       /* 前进/后退 */
            cs->TravelZ =  0;                           /* 无平移 */
            cs->TravelY = -(joy_delta(rx)) / 4;         /* 旋转 */
        }

        /* SELECT: 切换步态 (仅在静止时) */
        if (ps2_button_pressed(ps2, PSB_SELECT)) {
            if (cs->TravelX == 0 && cs->TravelY == 0 && cs->TravelZ == 0) {
                cs->GaitType++;
                if (cs->GaitType >= NUM_GAITS) cs->GaitType = 0;
                GaitSelect(cs->GaitType);
                ESP_LOGI(TAG, "Gait: %d", cs->GaitType);
            }
        }

        /* 行走模式下清除身体平移/旋转 (这些值在上一次转换/旋转模式下设置) */
        cs->BodyPosX = 0;
        cs->BodyPosZ = 0;
        cs->BodyRotX = 0;
        cs->BodyRotZ = 0;
        break;
    }

    case TRANSLATEMODE:
    {
        /* 左摇杆: 身体平移, 右摇杆 X: 旋转, 右摇杆 Y: 高度 */
        cs->BodyPosX  =  (joy_delta(ly)) / 3;     /* 身体 X 偏移 */
        cs->BodyPosZ  = -(joy_delta(lx)) / 2;     /* 身体 Z 偏移 */
        cs->BodyRotY  =  (joy_delta(rx)) * 2;     /* 身体 Yaw 旋转 */
        cs->BodyPosY += -(joy_delta(ry)) / 3;     /* 身体高度 */

        /* 限幅 */
        if (cs->BodyPosX > MAX_BODY_X) cs->BodyPosX = MAX_BODY_X;
        if (cs->BodyPosX < -MAX_BODY_X) cs->BodyPosX = -MAX_BODY_X;
        if (cs->BodyPosZ > MAX_BODY_Z) cs->BodyPosZ = MAX_BODY_Z;
        if (cs->BodyPosZ < -MAX_BODY_Z) cs->BodyPosZ = -MAX_BODY_Z;
        if (cs->BodyRotY > MAX_ROT_Y) cs->BodyRotY = MAX_ROT_Y;
        if (cs->BodyRotY < -MAX_ROT_Y) cs->BodyRotY = -MAX_ROT_Y;

        cs->TravelX = 0;
        cs->TravelY = 0;
        cs->TravelZ = 0;
        cs->BodyRotX = 0;
        cs->BodyRotZ = 0;
        break;
    }

    case ROTATEMODE:
    {
        /* 左摇杆 Y: Pitch, 左摇杆 X: Roll, 右摇杆 X: Yaw, 右摇杆 Y: 高度 */
        cs->BodyRotX  =  (joy_delta(ly));       /* Pitch */
        cs->BodyRotZ  =  (joy_delta(lx));       /* Roll */
        cs->BodyRotY  = -(joy_delta(rx)) * 2;   /* Yaw */
        cs->BodyPosY += -(joy_delta(ry)) / 3;   /* 高度 */

        if (cs->BodyRotX > MAX_ROT_X) cs->BodyRotX = MAX_ROT_X;
        if (cs->BodyRotX < -MAX_ROT_X) cs->BodyRotX = -MAX_ROT_X;
        if (cs->BodyRotZ > MAX_ROT_Z) cs->BodyRotZ = MAX_ROT_Z;
        if (cs->BodyRotZ < -MAX_ROT_Z) cs->BodyRotZ = -MAX_ROT_Z;
        if (cs->BodyRotY > MAX_ROT_Y) cs->BodyRotY = MAX_ROT_Y;
        if (cs->BodyRotY < -MAX_ROT_Y) cs->BodyRotY = -MAX_ROT_Y;

        cs->TravelX = 0;
        cs->TravelY = 0;
        cs->TravelZ = 0;
        cs->BodyPosX = 0;
        cs->BodyPosZ = 0;
        break;
    }

    case SINGLELEGMODE:
    {
        /* SELECT: 切换控制的腿 */
        if (ps2_button_pressed(ps2, PSB_SELECT)) {
            cs->SingleLegIndex++;
            if (cs->SingleLegIndex >= NUM_LEGS) cs->SingleLegIndex = 0;
            ESP_LOGI(TAG, "Single leg mode: leg %d", cs->SingleLegIndex);
        }

        /* R2: 保持/释放当前腿位置 */
        if (ps2_button_pressed(ps2, PSB_R2)) {
            cs->SingleLegHold = !cs->SingleLegHold;
            ESP_LOGI(TAG, "Single leg hold: %s", cs->SingleLegHold ? "ON" : "OFF");
        }

        /* 摇杆: 移动腿 */
        if (!cs->SingleLegHold) {
            cs->SingleLegX += joy_delta(lx) / 2;    /* 左右 */
            cs->SingleLegZ += joy_delta(ly) / 2;    /* 前后 */
            cs->SingleLegY += joy_delta(ry);        /* 高度 (绝对值) */
        }

        /* 行走参数归零 */
        cs->TravelX = 0;
        cs->TravelY = 0;
        cs->TravelZ = 0;
        cs->BodyPosX = 0;
        cs->BodyPosZ = 0;
        cs->BodyRotX = 0;
        cs->BodyRotY = 0;
        cs->BodyRotZ = 0;
        break;
    }

    case GPPLAYERMODE:
    default:
        cs->TravelX = 0;
        cs->TravelY = 0;
        cs->TravelZ = 0;
        break;
    }
}
