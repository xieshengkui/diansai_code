#ifndef __ENCODER_H
#define __ENCODER_H
#include "headfile.h"

typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float err;
    float err_sum;
    float err_diff;
    float last_err;
} PID;

#define FILTER_ALPHA       0.3f                   // 滤波系数
#define PID_OUTPUT_MAX     500                    // PID输出最大值
#define PID_OUTPUT_MIN     -500                   // PID输出最小值
#define INTEGRAL_MAX       10000                   // 积分上限
#define INTEGRAL_MIN       -10000                  // 积分下限
//另一个速度控制参数
//#define VELOCITY_KP        0.03f                   // 速度控制比例系数
//#define VELOCITY_KI        0.009f                  // 速度控制积分系数
#define VELOCITY_KP        0.1f                   // 速度控制比例系数
#define VELOCITY_KI        0.02f                  // 速度控制积分系数
#define VELOCITY_KD        0.0f                    // 速度控制微分系数
#define ENCODER_SAMPLING_INTERVAL_MS  10           // 编码器采样间隔（毫秒）
#define ENCODER_LINE_COUNT             13           // 编码器线数
#define ENCODER_REDUCTION_RATIO        20           // 减速比
#define ENCODER_MODE_MULTIPLIER        4           // 编码器模式倍数（AB双相4边沿计数）

extern PID Vpid_left;
extern PID Vpid_right;

int32_t limit_value(int32_t value, int32_t min, int32_t max);
void encoder_init(void);
int32_t encoder_get_count_left(void);
int32_t encoder_get_count_right(void);
double encoder_get_speed_left(void);
double encoder_get_speed_right(void);
int32_t velocity_pi_calc(PID *pid, double current_vel, double target_vel);
void encoder_print_speed(void);
void speed_control(float target_speed_left, float target_speed_right);


#endif