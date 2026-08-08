/**
 * @file stepper.h
 * @brief 串级PID 球杆平衡 — 外环位置 + 内环球速
 * @details 外环: pos_error(mm) → PID → 目标球速(mm/s)
 *          内环: 球速偏差 → PID → 水管角度 → 脉冲 → 电机
 *          核心思路: 球速越快越提前抵消倾斜, 实现主动刹车
 */

#ifndef __STEPPER_H_
#define __STEPPER_H_

#include "headfile.h"

/* ── K230 看门狗 ── */
#define K230_TIMEOUT_MS     200

// /* ── PID参数集: Q3循线 ── */
// #define Q3_POS_KP           4.5f
// #define Q3_POS_KI           0.01f
// #define Q3_POS_KD           10.0f
// #define Q3_VEL_KP           0.05f
// #define Q3_VEL_KI           0.0001f
// #define Q3_VEL_KD           0.55f
// #define Q3_ACC_KF           0.15f

/* ── PID参数集: Q3循线 ── */
#define Q3_POS_KP           1.5f
#define Q3_POS_KI           0.05f
#define Q3_POS_KD           10.0f
#define Q3_VEL_KP           0.15f
#define Q3_VEL_KI           0.001f
#define Q3_VEL_KD           0.0f
#define Q3_ACC_KF           0.2f

/* ── PID参数集: Q4循线 ── */
#define Q4_POS_KP           4.5f
#define Q4_POS_KI           0.01f
#define Q4_POS_KD           10.0f
#define Q4_VEL_KP           0.06f
#define Q4_VEL_KI           0.0001f
#define Q4_VEL_KD           0.5f
#define Q4_ACC_KF           0.15f

// /* ── PID参数集: Q4循线 ── */
// #define Q4_POS_KP           1.5f
// #define Q4_POS_KI           0.05f
// #define Q4_POS_KD           10.0f
// #define Q4_VEL_KP           0.15f
// #define Q4_VEL_KI           0.001f
// #define Q4_VEL_KD           0.0f
// #define Q4_ACC_KF           0.15f

// /* ── PID参数集: Q5循线 ── */
// #define Q5_POS_KP           4.5f
// #define Q5_POS_KI           0.01f
// #define Q5_POS_KD           10.0f
// #define Q5_VEL_KP           0.06f
// #define Q5_VEL_KI           0.0001f
// #define Q5_VEL_KD           0.5f
// #define Q5_ACC_KF           0.01f

/* ── PID参数集: Q5循线 ── */
#define Q5_POS_KP           1.5f
#define Q5_POS_KI           0.05f
#define Q5_POS_KD           10.0f
#define Q5_VEL_KP           0.015f
#define Q5_VEL_KI           0.001f
#define Q5_VEL_KD           0.0f
#define Q5_ACC_KF           0.01f

/* ── PID参数集2: 折返用 (小车静止, Q2) ── */
// #define STATIC_POS_KP       1.2f
// #define STATIC_POS_KI       0.022f
// #define STATIC_POS_KD       7.0f
// #define STATIC_VEL_KP       0.135f
// #define STATIC_VEL_KI       0.015f
// #define STATIC_VEL_KD       0.4f

#define STATIC_POS_KP       1.5f
#define STATIC_POS_KI       0.05f
#define STATIC_POS_KD       10.0f
#define STATIC_VEL_KP       0.15f
#define STATIC_VEL_KI       0.001f
#define STATIC_VEL_KD       0.0f

/* ── 默认使用折返套 (菜单界面稳定原点用) ── */
#define POS_KP              STATIC_POS_KP
#define POS_KI              STATIC_POS_KI
#define POS_KD              STATIC_POS_KD
#define VEL_KP              STATIC_VEL_KP
#define VEL_KI              STATIC_VEL_KI
#define VEL_KD              STATIC_VEL_KD

#define POS_INTEGRAL_MAX     300.0f
#define POS_INTEGRAL_MIN    -300.0f
#define POS_OUT_MAX          200.0f  /* 最大目标球速 mm/s */
#define POS_OUT_MIN         -200.0f

#define VEL_INTEGRAL_MAX     200.0f
#define VEL_INTEGRAL_MIN    -200.0f

/* ── 死区 ── */
// #define BALL_DEADBAND       0.0f    /* |pos_error| < 1mm */

/* ── 角度限位 ── */
#define ANGLE_MAX           25.0f
#define ANGLE_MIN          -25.0f

/* ── 脉冲换算 ── */
#define PULSES_PER_DEGREE   8.8889f /* 3200/360 */

/* ── 运动参数 ── */
#define STEPPER_POS_VEL     500
#define STEPPER_POS_ACC     80

/* ── 电机地址 ── */
#define STEPPER_ADDR        0x01

/* ── 外部变量 ── */
extern float ball_target_position;
extern float ball_velocity;          /* K230 'V': 小球实际速度 mm/s */
extern float ball_target_angle_debug;

/* ── API ── */
void stepper_init(void);
void stepper_write(uint8_t *data, uint16_t size);
void Emm_V5_Reset_Motor(uint8_t addr);
void Emm_V5_Reset_CurPos_To_Zero(uint8_t addr);
void Emm_V5_En_Control(uint8_t addr, bool state, bool snF);
void Emm_V5_Vel_Control_1(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);
void Emm_V5_Stop_Now(uint8_t addr, bool snF);
void Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, uint8_t raF, bool snF);

float ball_pd_calc(float error);
void  ball_balance_control(void);
void  ball_vel_control(float target_vel);   /* 纯速度环, 调内环PID用 */
void  stepper_goto_horizontal(void);
void  ball_pid_set_q3(void);
void  ball_pid_set_q4(void);
void  ball_pid_set_q5(void);
void  ball_pid_set_static(void);  /* 切换到折返套 (Q2) */

#endif
