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
    val |= ((DL_GPIO_readPins(TRACK_T1_PORT, TRACK_T1_PIN)!=0) << 11);
    val |= ((DL_GPIO_readPins(TRACK_T2_PORT, TRACK_T2_PIN)!=0) << 10);
    val |= ((DL_GPIO_readPins(TRACK_T3_PORT, TRACK_T3_PIN)!=0) << 9);
    val |= ((DL_GPIO_readPins(TRACK_T4_PORT, TRACK_T4_PIN)!=0) << 8);
    val |= ((DL_GPIO_readPins(TRACK_T5_PORT, TRACK_T5_PIN)!=0) << 7);
    val |= ((DL_GPIO_readPins(TRACK_T6_PORT, TRACK_T6_PIN)!=0) << 6);
    val |= ((DL_GPIO_readPins(TRACK_T7_PORT, TRACK_T7_PIN)!=0) << 5);
    val |= ((DL_GPIO_readPins(TRACK_T8_PORT, TRACK_T8_PIN)!=0) << 4);
    val |= ((DL_GPIO_readPins(TRACK_T9_PORT, TRACK_T9_PIN)!=0) << 3);
    val |= ((DL_GPIO_readPins(TRACK_T10_PORT, TRACK_T10_PIN)!=0) << 2);
    val |= ((DL_GPIO_readPins(TRACK_T11_PORT, TRACK_T11_PIN)!=0) << 1);
    val |= ((DL_GPIO_readPins(TRACK_T12_PORT, TRACK_T12_PIN)!=0) << 0);
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