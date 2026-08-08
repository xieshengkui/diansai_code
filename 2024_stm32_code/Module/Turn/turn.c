#include "headfile.h"

PID turn_pid={TURN_KP,TURN_KI,TURN_KD,0.0f,0.0f,0.0f};  // 转向PID控制器

// 静态变量用于累计距离
static double accumulated_distance = 0.0f;  // 累计行驶距离（厘米）

/**
 * @brief 计算最短角度差（自动处理-180~+180范围）
 * @param current 当前偏航角（-180~+180度）
 * @param target 目标偏航角（-180~+180度）
 * @return 最短角度差（-180~+180度）
 */
float turn_get_error(float current, float target) {
    float diff = target - current;
    while (diff > 180.0f)  diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    return diff;
}

/**
 * @brief 转向PD计算函数（无积分）
 * @param pid PID控制器指针
 * @param current_err 当前角度误差（-180~+180度）
 * @return PD输出值（带正负号）
 */
float turn_pd_calc(PID *pid, float current_err) {
    pid->err = current_err;
    
    // 计算微分项（无积分）
    pid->err_diff = pid->err - pid->last_err;
    pid->last_err = pid->err;
    
    // PD输出计算
    float output = pid->Kp * pid->err + pid->Kd * pid->err_diff;
    
    // 输出限幅
    if (output > TURN_OUTPUT_MAX) output = TURN_OUTPUT_MAX;
    if (output < TURN_OUTPUT_MIN) output = TURN_OUTPUT_MIN;
    
    return output;
}

/**
 * @brief 陀螺仪转向控制函数
 * @param base_speed 基础速度（RPM）
 * @param target_yaw 目标偏航角（度，范围-180~+180）
 */
void turn_control(float base_speed, float target_yaw) {
    // 1. 获取当前偏航角
    extern volatile float ypr[3];
    extern double current_speed_left;
    extern double current_speed_right;
    float current_yaw = ypr[0];
    // 2. 计算角度差（自动选择最短路径）
    float angle_err = turn_get_error(current_yaw, target_yaw);
    
    // 3. 判断是否到达目标角度（误差小于1度）
    if (fabs(angle_err) < 1.0f) {
        // 到达目标：按基础速度直行（不是停止）
        int16_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  base_speed);
        int16_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, base_speed);
        motor_set_speed(pwm_left, pwm_right);
        turn_pid.last_err = 0;  // 重置误差
        return;
    }
    
    // 4. PD控制计算转向调整量（steer_adjust本身带正负）
    float steer_adjust = turn_pd_calc(&turn_pid, angle_err);
    
    // 5. 简化差速转向计算（利用steer_adjust的正负）
    double target_left_speed  = base_speed + steer_adjust;
    double target_right_speed = base_speed - steer_adjust;
    
    // 6. 防反转限幅
    target_left_speed  = (target_left_speed  < 0) ? 0 : target_left_speed;
    target_right_speed = (target_right_speed < 0) ? 0 : target_right_speed;
    
    // 7. 速度环PI控制
    int16_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int16_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);
    
    // 8. 设置电机速度
    motor_set_speed(pwm_left, pwm_right);
}

/**
 * @brief 开环转向控制并返回行驶距离
 * @details 在控制方向的同时，通过编码器计数值计算并返回当前行驶距离
 * @param base_speed 基础速度（RPM）
 * @param target_yaw 目标偏航角（度，范围-180~+180）
 * @return 当前累计行驶距离（厘米），使用两轮计数值的平均值
 */
float turn_control_get_distance(float base_speed, float target_yaw) {
    // 1. 获取当前偏航角
    extern volatile float ypr[3];
    extern double current_speed_left;
    extern double current_speed_right;
    float current_yaw = ypr[0];
    
    // 2. 计算角度差（自动选择最短路径）
    float angle_err = turn_get_error(current_yaw, target_yaw);
    
    // 3. PD控制计算转向调整量（steer_adjust本身带正负）
    float steer_adjust = turn_pd_calc(&turn_pid, angle_err);
    
    // 4. 差速转向计算
    double target_left_speed  = base_speed + steer_adjust;
    double target_right_speed = base_speed - steer_adjust;
    
    // 5. 防反转限幅
    target_left_speed  = (target_left_speed  < 0) ? 0 : target_left_speed;
    target_right_speed = (target_right_speed < 0) ? 0 : target_right_speed;
    
    // 6. 速度环PI控制
    int16_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int16_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);
    
    // 7. 设置电机速度
    motor_set_speed(pwm_left, pwm_right);
    
    // 8. 获取编码器计数值（不清除，用于距离计算）
    int32_t count_left = encoder_get_count_left();
    int32_t count_right = encoder_get_count_right();
    
    // 9. 计算轮子周长（厘米）
    float wheel_circum = 2.0f * PI * WHEEL_RADIUS_CM;
    
    // 10. 计算本次采样的距离增量（使用两轮平均值）
    // 距离 = (计数值 / (编码器线数 * 减速比 * 倍频)) * 轮子周长
    double delta_left = count_left * wheel_circum / 
                       (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    double delta_right = count_right * wheel_circum / 
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    
    // 11. 累加距离（两轮平均值）
    accumulated_distance += (delta_left + delta_right) / 2.0f;
    
    // 12. 返回累计距离
    return accumulated_distance;
}

/**
 * @brief 重置累计距离为零
 * @details 用于在开始新的移动任务前重置距离计数器
 */
void turn_control_reset_distance(void) {
    accumulated_distance = 0.0f;
}