#include "IK.h"
#include "robot_config.h"
#include <math.h>

/* ===================================================================
 *  LegIK — 单腿逆运动学
 *
 *  输入:  足端在腿局部坐标系的目标坐标 (x, y, z)  (mm)
 *  输出:  Coxa (基节), Femur (股节), Tibia (胫节)  (rad)
 *
 *  算法:  解析 Law of Cosines
 *
 *  坐标系 (腿局部):
 *    X: 前方 (前进方向)
 *    Y: 上方 (抬腿方向)
 *    Z: 侧方 (远离身体方向)
 * =================================================================== */
bool LegIK(float x, float y, float z,
           float *coxa,
           float *femur,
           float *tibia)
{
    /* Step 1: Coxa angle — horizontal rotation to point at target */
    *coxa = atan2f(x, z);

    /* Step 2: Project to planar distance from femur pivot */
    float L = sqrtf(x*x + z*z) - COXA_LENGTH;
    float D = sqrtf(L*L + y*y);

    /* Sanity check: target within reach? */
    if (D > (FEMUR_LENGTH + TIBIA_LENGTH)) {
        /* Target too far, clamp */
        D = FEMUR_LENGTH + TIBIA_LENGTH - 1.0f;
    }
    if (D < fabsf(FEMUR_LENGTH - TIBIA_LENGTH)) {
        /* Target too close to body */
        D = fabsf(FEMUR_LENGTH - TIBIA_LENGTH) + 1.0f;
    }

    /* Step 3: Femur angle */
    float a1 = atan2f(y, L);
    float a2 = acosf(
        (FEMUR_LENGTH*FEMUR_LENGTH + D*D - TIBIA_LENGTH*TIBIA_LENGTH) /
        (2.0f * FEMUR_LENGTH * D)
    );
    *femur = a1 + a2;

    /* Step 4: Tibia angle */
    *tibia = acosf(
        (FEMUR_LENGTH*FEMUR_LENGTH + TIBIA_LENGTH*TIBIA_LENGTH - D*D) /
        (2.0f * FEMUR_LENGTH * TIBIA_LENGTH)
    ) - (float)M_PI;

    return true;
}

/* ===================================================================
 *  BodyIK — 身体逆运动学
 *
 *  将身体姿态 (平移 + 旋转) 转换为每条腿的足端目标坐标。
 *
 *  步骤:
 *    1. 计算足端在全局坐标系的位置 = 腿安装偏移 + 身体平移 + 步态偏移
 *    2. 应用身体旋转的逆矩阵 (R^T)
 *    3. 减去腿安装偏移, 得到腿局部坐标系的足端目标
 *
 *  坐标系:
 *    X: 前方 (+), 后方 (-)
 *    Y: 上方 (+), 下方 (-)
 *    Z: 右方 (+), 左方 (-)  [右腿 Z 偏移为负, 左腿 Z 偏移为正]
 *
 *  旋转顺序: Roll(X) → Pitch(Y) → Yaw(Z)
 *  逆旋转:    Yaw⁻¹ → Pitch⁻¹ → Roll⁻¹
 * =================================================================== */

/* 腿安装偏移查找表 */
typedef struct { float x, z; } LegOffset_t;

static const LegOffset_t LegOffsets[NUM_LEGS] = {
    /* 0=RR */ { LEG_RR_OFFSET_X, LEG_RR_OFFSET_Z },
    /* 1=RM */ { LEG_RM_OFFSET_X, LEG_RM_OFFSET_Z },
    /* 2=RF */ { LEG_RF_OFFSET_X, LEG_RF_OFFSET_Z },
    /* 3=LR */ { LEG_LR_OFFSET_X, LEG_LR_OFFSET_Z },
    /* 4=LM */ { LEG_LM_OFFSET_X, LEG_LM_OFFSET_Z },
    /* 5=LF */ { LEG_LF_OFFSET_X, LEG_LF_OFFSET_Z },
};

void BodyIK(uint8_t leg,
            const BodyPose_t *pose,
            float gait_x, float gait_y, float gait_z,
            LegTarget_t *target)
{
    if (leg >= NUM_LEGS) return;

    float offset_x = LegOffsets[leg].x;
    float offset_z = LegOffsets[leg].z;

    /* 1. 全局坐标系中的足端位置 (旋转前) */
    float wx = offset_x + pose->BodyPosX + gait_x;
    float wy = /* offset_y=0 */ pose->BodyPosY + gait_y;
    float wz = offset_z + pose->BodyPosZ + gait_z;

    /* 2. 身体旋转角 (度 → 弧度) */
    float roll  = DEG_TO_RAD(pose->BodyRotX);   /* Roll (X 轴) */
    float pitch = DEG_TO_RAD(pose->BodyRotY);   /* Pitch (Y 轴) */
    float yaw   = DEG_TO_RAD(pose->BodyRotZ);   /* Yaw (Z 轴) */

    float cr = cosf(roll),  sr = sinf(roll);
    float cp = cosf(pitch), sp = sinf(pitch);
    float cy = cosf(yaw),   sy = sinf(yaw);

    /* 3. 逆旋转: 将全局坐标变换回身体坐标系 */
    /*
     * R = Rz(yaw) * Ry(pitch) * Rx(roll)
     * R^T = Rx(-roll) * Ry(-pitch) * Rz(-yaw)
     *
     * Step A: Un-yaw (rotate by -yaw around Z)
     *   x1 =  cy * wx + sy * wz
     *   z1 = -sy * wx + cy * wz
     *   y1 = wy
     */
    float x1 =  cy * wx + sy * wz;
    float z1 = -sy * wx + cy * wz;
    float y1 = wy;

    /*
     * Step B: Un-pitch (rotate by -pitch around Y)
     *   x2 = cp * x1 - sp * y1
     *   y2 = sp * x1 + cp * y1
     *   z2 = z1
     */
    float x2 = cp * x1 - sp * y1;
    float y2 = sp * x1 + cp * y1;
    float z2 = z1;

    /*
     * Step C: Un-roll (rotate by -roll around X)
     *   x3 = x2
     *   y3 =  cr * y2 + sr * z2
     *   z3 = -sr * y2 + cr * z2
     */
    float x3 = x2;
    float y3 =  cr * y2 + sr * z2;
    float z3 = -sr * y2 + cr * z2;

    /* 4. 腿局部坐标系 = 逆旋转后的位置 - 腿安装偏移 */
    target->x = x3 - offset_x;
    target->y = y3;
    target->z = z3 - offset_z;
}
