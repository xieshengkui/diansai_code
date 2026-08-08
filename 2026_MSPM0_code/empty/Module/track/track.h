#ifndef __TRACK_H
#define __TRACK_H
#include "headfile.h"
#include "encoder.h"

int track_read(void);
float track_get_error(void);
float track_pd_calc(PID *pid, float current_err);
void track_control(float base_speed);
float track_control_get_distance(float base_speed);
void track_control_reset_distance(void);
uint8_t track_detect_middle4(void);
uint8_t track_detect_middle4_cached(void);
uint8_t track_detect_start_line_cached(void);  // 8路全检, ≥4路黑=启停线

// #define STEER_KP           16.0f   // 转向比例系数
// #define STEER_KI           0.0f    // 转向积分系数
// #define STEER_KD           20.0f  // 转向微分系数

#define STEER_KP           7.0f   // 转向比例系数
#define STEER_KI           0.0f    // 转向积分系数
#define STEER_KD           20.0f  // 转向微分系数
#define STEER_OUTPUT_MAX   100.0f  // 最大转向输出
#define STEER_OUTPUT_MIN   -100.0f // 最小转向输出

#define SMOOTH_MIN_START_SPEED 10.0f   // 起步最低速度 (RPM)，克服静摩擦
#define SMOOTH_MIN_STOP_SPEED   15.0f   // 停车最低速度 (RPM)，低于此值直接归0

/* ── Q1 循线转向PID (独立参数) ── */
#define STEER_KP_Q1          20.0f
#define STEER_KI_Q1          0.0f
#define STEER_KD_Q1          40.0f
#define STEER_OUTPUT_MAX_Q1  100.0f00
#define STEER_OUTPUT_MIN_Q1 -100.0f

extern PID track_pid;
extern PID track_pid_q1;
void track_control_q1(float base_speed);
float track_control_q1_get_distance(float base_speed);

/* ── 速度剖面 API ── */
float speed_profile_trapezoid(float current_dist, float total_dist,
                               float max_speed, float ramp_ratio);
float track_control_smooth(float total_dist, float max_speed, float ramp_ratio);

/* ── 不对称梯形剖面: 加速/减速独立控制 ── */
float speed_profile_asymmetric(float current_dist, float total_dist,
                                float max_speed, float accel_dist, float decel_dist);
float track_control_asymmetric(float total_dist, float max_speed,
                                float accel_dist, float decel_dist);

#endif