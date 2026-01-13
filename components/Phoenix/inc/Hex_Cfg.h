#pragma once

/* ================= 机器人几何参数 ================= */

// 舵机数量
#define NUM_LEGS            6
#define JOINTS_PER_LEG      3

// 单位：mm
#define COXA_LENGTH         52.0f
#define FEMUR_LENGTH        82.0f
#define TIBIA_LENGTH        140.0f

// 身体尺寸
#define BODY_X              75.0f
#define BODY_Y              60.0f

/* ================= 步态参数 ================= */

#define DEFAULT_GAIT_SPEED  60     // ms per step
#define LEG_LIFT_HEIGHT     30.0f  // mm
#define LEG_LIFT_DIV        2

/* ================= 运动限制 ================= */

#define MAX_BODY_Y          50.0f
#define MAX_BODY_X          50.0f
#define MAX_BODY_Z          50.0f

#define MAX_ROT_X           30.0f
#define MAX_ROT_Y           30.0f
#define MAX_ROT_Z           30.0f

/* ================= 数学宏 ================= */

#define DEG_TO_RAD(x)       ((x) * 0.01745329252f)
#define RAD_TO_DEG(x)       ((x) * 57.29577951f)

/* ================= 调试 ================= */

#define PHOENIX_DEBUG       1
