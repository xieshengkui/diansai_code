#ifndef __MOTOR_H
#define __MOTOR_H
#include "headfile.h"

void motor_init();
void motor_set_left_speed(int32_t CCR_num);
void motor_set_right_speed(int32_t CCR_num);
void motor_set_speed(int32_t left_num, int32_t right_num);

#endif