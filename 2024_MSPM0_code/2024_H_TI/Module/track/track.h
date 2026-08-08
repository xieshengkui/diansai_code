#ifndef __TRACK_H
#define __TRACK_H
#include "headfile.h"
#include "encoder.h"

int track_read(void);
float track_get_error(void);
float track_pd_calc(PID *pid, float current_err);
void track_control(float base_speed);

#define STEER_KP           25.0f   // 转向比例系数
#define STEER_KI           0.0f    // 转向积分系数
#define STEER_KD           250.0f  // 转向微分系数
#define STEER_OUTPUT_MAX   200.0f  // 最大转向输出
#define STEER_OUTPUT_MIN   -200.0f // 最小转向输出

extern PID track_pid;

#endif