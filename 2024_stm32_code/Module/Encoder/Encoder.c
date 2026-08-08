/**
 * @file Encoder.c
 * @brief 编码器模块实现
 * @details 实现编码器初始化、速度计算和PID控制相关功能
 */
#include "headfile.h"

/**
 * @brief 限幅函数
 * @param value 输入值
 * @param min 最小值
 * @param max 最大值
 * @return 限幅后的值
 */
int32_t limit_value(int32_t value, int32_t min, int32_t max) {
    if(value > max) return max;
    if(value < min) return min;
    return value;
}

/**
 * @brief 编码器初始化函数
 * @details 初始化编码器定时器和采样定时器
 */
void encoder_init(void) {
    // 启动TIM4（左轮编码器）和TIM5（右轮编码器）的编码器模式
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Base_Start_IT(&htim6);  // 启动定时器中断
}

/**
 * @brief 获取左轮编码器计数值
 * @return 左轮编码器的当前计数值
 */
int32_t encoder_get_count_left(void)  { return (int32_t)(int16_t)TIM5->CNT; }

/**
 * @brief 获取右轮编码器计数值
 * @return 右轮编码器的当前计数值
 */
int32_t encoder_get_count_right(void) { return (int32_t)(int16_t)TIM4->CNT; }

// 速度全局变量
double velocity_left = 0.0f;           // 左轮原始速度s
double filt_velocity_left = 0.0f;      // 左轮滤波后速度
double last_filt_velocity_left = 0.0f; // 左轮上一次滤波速度
double velocity_right = 0.0f;          // 右轮原始速度
double filt_velocity_right = 0.0f;     // 右轮滤波后速度
double last_filt_velocity_right = 0.0f; // 右轮上一次滤波速度   

/**
 * @brief 计算左轮速度
 * @return 左轮当前速度，单位：RPM
 */
double encoder_get_speed_left(void) {
    // 获取左轮编码器计数值
    int32_t left_count = encoder_get_count_left();
    // 重置计数器
    TIM5->CNT = 0;
    
    // 计算速度：(1000ms * 60s * 计数) / (编码器线数 * 减速比 * 倍频 * 采样时间)
    velocity_left = (1000.0 * 60.0 * left_count) /
           (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER * ENCODER_SAMPLING_INTERVAL_MS);
    
    // 一阶低通滤波
    filt_velocity_left = FILTER_ALPHA * velocity_left + (1 - FILTER_ALPHA) * last_filt_velocity_left;
    last_filt_velocity_left = filt_velocity_left;
    
    return filt_velocity_left;
}

/**
 * @brief 计算右轮速度
 * @return 右轮当前速度，单位：RPM
 */
double encoder_get_speed_right(void) {
    // 获取右轮编码器计数值
    int32_t right_count = encoder_get_count_right();
    // 重置计数器
    TIM4->CNT = 0;
    
    // 计算速度：(1000ms * 60s * 计数) / (编码器线数 * 减速比 * 倍频 * 采样时间)
    velocity_right = (1000.0 * 60.0 * right_count) /
           (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER * ENCODER_SAMPLING_INTERVAL_MS);
    
    // 一阶低通滤波
    filt_velocity_right = FILTER_ALPHA * velocity_right + (1 - FILTER_ALPHA) * last_filt_velocity_right;
    last_filt_velocity_right = filt_velocity_right;
    
    return filt_velocity_right;
}

// 速度环PID实例
PID Vpid_left  = {VELOCITY_KP, VELOCITY_KI, VELOCITY_KD, 0.0f, 0.0f, 0.0f, 0.0f};  // 左轮速度PID
PID Vpid_right = {VELOCITY_KP, VELOCITY_KI, VELOCITY_KD, 0.0f, 0.0f, 0.0f, 0.0f};  // 右轮速度PID

/**
 * @brief 速度环PI计算
 * @param pid PI控制器实例
 * @param current_vel 当前速度
 * @param target_vel 目标速度
 * @return 控制输出值
 */
int32_t velocity_pi_calc(PID *pid, double current_vel, double target_vel) {
    // 计算当前误差
    pid->err = target_vel - current_vel;
    // 计算误差积分
    pid->err_sum += pid->err;
    // 积分限幅，防止积分饱和
    pid->err_sum = limit_value(pid->err_sum, INTEGRAL_MIN, INTEGRAL_MAX);
    // 计算PI输出
    int32_t output = (int32_t)(pid->Kp * pid->err + pid->Ki * pid->err_sum);
    // 输出限幅
    return limit_value(output, PID_OUTPUT_MIN, PID_OUTPUT_MAX);
}

void speed_control(float target_speed_left, float target_speed_right) {
    // 获取当前速度
    extern double current_speed_left;
    extern double current_speed_right;
    
    // 计算控制输出
    int32_t pwm_left  = velocity_pi_calc(&Vpid_left, current_speed_left, target_speed_left);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_speed_right);
    
    // 设置电机速度
    motor_set_speed(pwm_left, pwm_right);
}
