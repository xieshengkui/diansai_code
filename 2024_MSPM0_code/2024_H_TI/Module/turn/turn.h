#ifndef __TURN_H
#define __TURN_H
#include "headfile.h"

#define TURN_KP        1.5f                   // 转向控制比例系数
#define TURN_KI        0.0f                  // 转向控制积分系数
#define TURN_KD        1.5f                    // 转向控制微分系数
#define TURN_OUTPUT_MAX 150.0f  // 最大转向输出
#define TURN_OUTPUT_MIN -150.0f // 最小转向输出

extern PID turn_pid;

float turn_get_error(float current, float target);
float turn_pd_calc(PID *pid, float current_err);
void  turn_control(float base_speed, float target_yaw);
float turn_control_get_distance(float base_speed, float target_yaw);
void turn_control_reset_distance(void);


#endif