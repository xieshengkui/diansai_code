/**
 * @file track.c
 * @brief 循迹模块实现
 * @details 实现灰度传感器读取、偏差计算和转向控制相关功能
 */
#include "headfile.h"

// 转向环PD控制器实例
PID track_pid = {STEER_KP, STEER_KI, STEER_KD, 0.0f, 0.0f, 0.0f, 0.0f};

/**
 * @brief 读取12路灰度传感器状态
 * @return 12位二进制数，每一位代表一个传感器的状态
 *         0表示检测到白色地面，1表示检测到黑线
 */
int track_read(void) {
    int val = 0;
    // 读取12路灰度传感器状态，从TRACK1到TRACK12
    val |= (HAL_GPIO_ReadPin(TRACK1_GPIO_Port,  TRACK1_Pin)  << 11);
    val |= (HAL_GPIO_ReadPin(TRACK2_GPIO_Port,  TRACK2_Pin)  << 10);
    val |= (HAL_GPIO_ReadPin(TRACK3_GPIO_Port,  TRACK3_Pin)  << 9);
    val |= (HAL_GPIO_ReadPin(TRACK4_GPIO_Port,  TRACK4_Pin)  << 8);
    val |= (HAL_GPIO_ReadPin(TRACK5_GPIO_Port,  TRACK5_Pin)  << 7);
    val |= (HAL_GPIO_ReadPin(TRACK6_GPIO_Port,  TRACK6_Pin)  << 6);
    val |= (HAL_GPIO_ReadPin(TRACK7_GPIO_Port,  TRACK7_Pin)  << 5);
    val |= (HAL_GPIO_ReadPin(TRACK8_GPIO_Port,  TRACK8_Pin)  << 4);
    val |= (HAL_GPIO_ReadPin(TRACK9_GPIO_Port,  TRACK9_Pin)  << 3);
    val |= (HAL_GPIO_ReadPin(TRACK10_GPIO_Port, TRACK10_Pin) << 2);
    val |= (HAL_GPIO_ReadPin(TRACK11_GPIO_Port, TRACK11_Pin) << 1);
    val |= (HAL_GPIO_ReadPin(TRACK12_GPIO_Port, TRACK12_Pin) << 0);
    return val;
}

/**
 * @brief 计算循迹黑线的位置偏差值
 * @return 偏差值，范围[-5.5, 5.5]
 *         0表示在黑线中心，正值表示偏右，负值表示偏左
 */
float track_get_error(void) {
    int val = track_read();
    int count = 0;
    float sum_pos = 0.0f;

    // 计算检测到黑线的传感器位置总和
    if( val & (1 << 11) ) { sum_pos += 0.0f; count++; }  // TRACK1
    if( val & (1 << 10) ) { sum_pos += 1.0f; count++; }  // TRACK2
    if( val & (1 << 9) )  { sum_pos += 2.0f; count++; }  // TRACK3
    if( val & (1 << 8) )  { sum_pos += 3.0f; count++; }  // TRACK4
    if( val & (1 << 7) )  { sum_pos += 4.0f; count++; }  // TRACK5
    if( val & (1 << 6) )  { sum_pos += 5.0f; count++; }  // TRACK6
    if( val & (1 << 5) )  { sum_pos += 6.0f; count++; }  // TRACK7
    if( val & (1 << 4) )  { sum_pos += 7.0f; count++; }  // TRACK8
    if( val & (1 << 3) )  { sum_pos += 8.0f; count++; }  // TRACK9
    if( val & (1 << 2) )  { sum_pos += 9.0f; count++; }  // TRACK10
    if( val & (1 << 1) )  { sum_pos += 10.0f; count++; } // TRACK11
    if( val & (1 << 0) )  { sum_pos += 11.0f; count++; } // TRACK12

    // 如果没有检测到黑线，返回0
    if(count == 0) return 0.0f;
    // 计算平均位置并减去中心点（5.5）得到偏差值
    return (sum_pos / count) - 5.5f;
}

/**
 * @brief 转向环PD控制器计算
 * @param pid PD控制器实例
 * @param current_err 当前偏差值
 * @return 转向调整量
 */
float track_pd_calc(PID *pid, float current_err) {
    pid->err = current_err;
    // 计算误差微分
    pid->err_diff = pid->err - pid->last_err;
    // 保存当前误差作为下一次的上一次误差
    pid->last_err = pid->err;

    // 计算PD输出
    float output = pid->Kp * pid->err + pid->Kd * pid->err_diff;
    // 输出限幅
    output = (output > STEER_OUTPUT_MAX) ? STEER_OUTPUT_MAX : output;
    output = (output < STEER_OUTPUT_MIN) ? STEER_OUTPUT_MIN : output;
    return output;
}


float steer_err_printf;
/**
 * @brief 循迹控制主函数（整合版）
 * @details 包含完整的循迹控制流程：
 *          1. 读取循迹传感器
 *          2. 计算偏差和目标速度
 *          3. 速度环PI控制
 *          4. 设置电机输出
 * @param base_speed 基础速度值（RPM）
 */
void track_control(float base_speed) {
    extern double current_speed_left;
    extern double current_speed_right;
    // ========== 1. 读取传感器并计算偏差 ==========
    float steer_err = track_get_error();
    steer_err_printf = steer_err; // 用于调试输出
    // ========== 2. 计算目标速度（转向环PD） ==========
    double target_left_speed;
    double target_right_speed;
    
    
    // 正常循迹：PD控制计算转向调整量
    float steer_adjust = track_pd_calc(&track_pid, steer_err);
        
    // 差速转向
    target_left_speed  = base_speed - steer_adjust;
    target_right_speed = base_speed + steer_adjust;
        
    // 防反转限幅
    target_left_speed  = (target_left_speed < 0)  ? 0 : target_left_speed;
    target_right_speed = (target_right_speed < 0) ? 0 : target_right_speed;
    

    // 计算电机PWM输出
    int16_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int16_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);
    
    // ========== 4. 设置电机速度 ==========
    motor_set_speed(pwm_left, pwm_right);
}