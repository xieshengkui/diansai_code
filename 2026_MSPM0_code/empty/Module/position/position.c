#include "headfile.h"

PID position_pid_left  = {POSITION_KP, 0.0f, POSITION_KD, 0.0f, 0.0f, 0.0f, 0.0f};  // 左轮位置环PID控制器（无积分）
PID position_pid_right = {POSITION_KP, 0.0f, POSITION_KD, 0.0f, 0.0f, 0.0f, 0.0f};  // 右轮位置环PID控制器（无积分）

float current_position_left = 0.0f;   // 左轮累计位置（厘米）
float current_position_right = 0.0f;  // 右轮累计位置（厘米）
float target_position = 0.0f;         // 目标位置（厘米）

/**
 * @brief 位置环PD计算函数
 * @param pid PID控制器指针
 * @param current_pos 当前位置（厘米）
 * @param target_pos 目标位置（厘米）
 * @return PD输出值（目标速度，RPM）
 */
float position_pd_calc(PID *pid, float current_pos, float target_pos) {
    // 计算位置误差
    pid->err = target_pos - current_pos;
    
    // 计算微分项
    pid->err_diff = pid->err - pid->last_err;
    pid->last_err = pid->err;
    
    // PD输出计算
    float output = pid->Kp * pid->err + pid->Kd * pid->err_diff;

    // 输出限幅
    return limit_value_float(output, POSITION_OUTPUT_MIN, POSITION_OUTPUT_MAX);
}

/**
 * @brief 更新位置（根据编码器计数值计算并累加位置）
 * @details 每隔10ms调用一次，获取编码器计数值并计算位置增量
 */
void position_update(void) {
    // 获取编码器计数值（由 encoder_get_speed_left 清零，此处只读不重置）
    int32_t count_left = encoder_get_count_left();
    int32_t count_right = encoder_get_count_right();
    
    // 计算本次采样的位置增量（厘米）
    // 距离 = (计数值 / (编码器线数 * 减速比 * 倍频)) * 轮子周长
    float delta_left = count_left * WHEEL_CIRCUMFERENCE_CM / 
                       (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    float delta_right = count_right * WHEEL_CIRCUMFERENCE_CM / 
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    
    // 累加位置
    current_position_left += delta_left;
    current_position_right += delta_right;
}

/**
 * @brief 位置控制函数
 * @param target_pos 目标位置（厘米），支持正负值（正为前进，负为后退）
 * @details 位置环控制流程：
 *          1. 更新左右轮当前位置
 *          2. 分别计算左右轮位置误差（保留正负号，支持双向控制）
 *          3. 判断是否到达目标位置（两轮都到达才算到达）
 *          4. 左右轮独立进行PD控制，计算各自目标速度
 *          5. 通过速度环PI控制输出PWM到电机
 */
void position_control(float target_pos) {
    extern float current_speed_left;
    extern float current_speed_right;
    // 更新目标位置全局变量
    target_position = target_pos;
    
    // 1. 更新当前位置（读取编码器并累加距离）
    position_update();
    
    // 2. 计算左右轮位置误差（保留正负号，支持双向控制）
    //    正误差：当前位置 < 目标位置，需要前进
    //    负误差：当前位置 > 目标位置，需要后退
    float pos_err_left = target_pos - current_position_left;
    float pos_err_right = target_pos - current_position_right;
    
    // 3. 判断是否到达目标位置（两轮误差都小于0.5厘米才算到达）
    //    使用fabsf只用于判断是否到达，不影响控制方向
    if (fabsf(pos_err_left) < 0.5f && fabsf(pos_err_right) < 0.5f) {
        // 到达目标：设置目标速度为0，停止电机
        int32_t pwm_left = velocity_pi_calc(&Vpid_left, current_speed_left, 0.0f);
        int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, 0.0f);
        motor_set_speed(pwm_left, pwm_right);
        position_pid_left.last_err = 0.0f;   // 重置左轮PD控制器的上一次误差
        position_pid_right.last_err = 0.0f;  // 重置右轮PD控制器的上一次误差
        return;  // 直接返回，无需继续控制
    }
    
    // 4. 位置环PD控制：左右轮独立计算目标速度
    //    左轮根据左轮位置计算目标速度
    float target_speed_left = position_pd_calc(&position_pid_left, current_position_left, target_pos);

    //    右轮根据右轮位置计算目标速度
    float target_speed_right = position_pd_calc(&position_pid_right, current_position_right, target_pos);
    
    // 5. 速度环PI控制：根据目标速度计算PWM输出
    int32_t pwm_left = velocity_pi_calc(&Vpid_left, current_speed_left, target_speed_left);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_speed_right);
    
    // 6. 设置电机PWM，执行控制
    motor_set_speed(pwm_left, pwm_right);
}

/**
 * @brief 重置位置为零
 */
void position_reset(void) {
    current_position_left = 0.0f;
    current_position_right = 0.0f;
    position_pid_left.err = 0.0f;
    position_pid_left.err_diff = 0.0f;
    position_pid_left.last_err = 0.0f;
    position_pid_right.err = 0.0f;
    position_pid_right.err_diff = 0.0f;
    position_pid_right.last_err = 0.0f;
}