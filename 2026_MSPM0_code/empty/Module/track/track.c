/**
 * @file track.c
 * @brief 循迹模块实现
 * @details 实现灰度传感器读取、偏差计算和转向控制相关功能
 */
#include "headfile.h"


// 转向环PD控制器实例
PID track_pid    = {STEER_KP,    STEER_KI,    STEER_KD,    0.0f, 0.0f, 0.0f, 0.0f};
PID track_pid_q1 = {STEER_KP_Q1, STEER_KI_Q1, STEER_KD_Q1, 0.0f, 0.0f, 0.0f, 0.0f};

/* ── 文件级静态变量 ── */
static float steer_err_printf;
static int   g_last_track_val;  // 缓存最近一次 track_read() 原始值

/**
 * @brief 读取8路灰度传感器状态 (直接IO模式)
 * @details 8路传感器各接独立GPIO: X1~X8
 *          X1=物理左侧(bit0), X8=物理右侧(bit7)
 * @return 8位二进制数, bit=0检测到黑线, bit=1检测到白色地面
 */
int track_read(void) {
    int val = 0;

    if (DL_GPIO_readPins(TRACK_X1_PORT, TRACK_X1_PIN) != 0) val |= (1 << 0);
    if (DL_GPIO_readPins(TRACK_X2_PORT, TRACK_X2_PIN) != 0) val |= (1 << 1);
    if (DL_GPIO_readPins(TRACK_X3_PORT, TRACK_X3_PIN) != 0) val |= (1 << 2);
    if (DL_GPIO_readPins(TRACK_X4_PORT, TRACK_X4_PIN) != 0) val |= (1 << 3);
    if (DL_GPIO_readPins(TRACK_X5_PORT, TRACK_X5_PIN) != 0) val |= (1 << 4);
    if (DL_GPIO_readPins(TRACK_X6_PORT, TRACK_X6_PIN) != 0) val |= (1 << 5);
    if (DL_GPIO_readPins(TRACK_X7_PORT, TRACK_X7_PIN) != 0) val |= (1 << 6);
    if (DL_GPIO_readPins(TRACK_X8_PORT, TRACK_X8_PIN) != 0) val |= (1 << 7);

    return val;
}

/**
 * @brief 检测启停线 (中间6路至少4路检测到黑线)
 * @return 1=检测到启停线, 0=未检测到
 * @details 中间6路(通道2-7, bit1-6)至少4路为黑时判定为启停线
 */
uint8_t track_detect_middle4(void) {
    int val = track_read();
    uint8_t bits = (val >> 1) & 0x3F;   /* bit1~bit6 = 中间6路 */
    uint8_t cnt = 0;
    if (!(bits & 0x01)) cnt++;  // 0=黑线
    if (!(bits & 0x02)) cnt++;
    if (!(bits & 0x04)) cnt++;
    if (!(bits & 0x08)) cnt++;
    if (!(bits & 0x10)) cnt++;
    if (!(bits & 0x20)) cnt++;
    return cnt >= 4;
}

/**
 * @brief 检测启停线 (复用 g_last_track_val，不重复读传感器)
 */
uint8_t track_detect_middle4_cached(void) {
    uint8_t bits = (g_last_track_val >> 1) & 0x3F;
    uint8_t cnt = 0;
    if (!(bits & 0x01)) cnt++;
    if (!(bits & 0x02)) cnt++;
    if (!(bits & 0x04)) cnt++;
    if (!(bits & 0x08)) cnt++;
    if (!(bits & 0x10)) cnt++;
    if (!(bits & 0x20)) cnt++;
    return cnt >= 4;
}

/**
 * @brief 检测启停线 (8路全检, ≥4路黑线)
 * @details 复用 g_last_track_val, 在 track_get_error() 之后调用
 */
uint8_t track_detect_start_line_cached(void) {
    int val = g_last_track_val;
    uint8_t cnt = 0;
    if (!(val & (1 << 0))) cnt++;
    if (!(val & (1 << 1))) cnt++;
    if (!(val & (1 << 2))) cnt++;
    if (!(val & (1 << 3))) cnt++;
    if (!(val & (1 << 4))) cnt++;
    if (!(val & (1 << 5))) cnt++;
    if (!(val & (1 << 6))) cnt++;
    if (!(val & (1 << 7))) cnt++;
    return cnt >= 4;
}

/**
 * @brief 计算循迹黑线的位置偏差值
 * @return 偏差值，范围[-3.5, 3.5]
 *         0表示在黑线中心，正值表示偏右，负值表示偏左
 */
float track_get_error(void) {
    int val = track_read();
    g_last_track_val = val;  // 缓存原始值, 供 stop-line 检测复用
    int count = 0;
    float sum_pos = 0.0f;

    // 计算检测到黑线的传感器位置总和（0=黑线, 1=白地）
    // 通道顺序与物理位置一致：通道1在物理左侧，通道8在物理右侧
    if( !(val & (1 << 0)) )  { sum_pos += 0.0f; count++; }  // 通道1（物理左侧）
    if( !(val & (1 << 1)) )  { sum_pos += 1.0f; count++; }  // 通道2
    if( !(val & (1 << 2)) )  { sum_pos += 2.0f; count++; }  // 通道3
    if( !(val & (1 << 3)) )  { sum_pos += 3.0f; count++; }  // 通道4
    if( !(val & (1 << 4)) )  { sum_pos += 4.0f; count++; }  // 通道5
    if( !(val & (1 << 5)) )  { sum_pos += 5.0f; count++; }  // 通道6
    if( !(val & (1 << 6)) )  { sum_pos += 6.0f; count++; }  // 通道7
    if( !(val & (1 << 7)) )  { sum_pos += 7.0f; count++; }  // 通道8（物理右侧）

    // 如果没有检测到黑线，返回0
    if(count == 0) return 0.0f;
    // 计算平均位置并减去中心点（3.5）得到偏差值
    return (sum_pos / count) - 3.5f;
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


// 静态变量用于累计行驶距离
static float track_accumulated_distance = 0.0f;

/**
 * @brief 循迹控制并返回行驶距离
 * @details 在循迹控制的同时，通过编码器计数值计算并返回当前行驶距离
 * @param base_speed 基础速度值（RPM）
 * @return 当前累计行驶距离（厘米），使用两轮计数值的平均值
 */
float track_control_get_distance(float base_speed) {
    extern float current_speed_left;
    extern float current_speed_right;

    // 1. 读取传感器并计算偏差
    float steer_err = track_get_error();
    steer_err_printf = steer_err;

    // 2. 计算目标速度（转向环PD）
    float steer_adjust = track_pd_calc(&track_pid, steer_err);

    // 3. 差速转向
    float target_left_speed  = base_speed + steer_adjust;
    float target_right_speed = base_speed - steer_adjust;

    // 4. 防反转限幅
    target_left_speed  = (target_left_speed  < 0) ? 0 : target_left_speed;
    target_right_speed = (target_right_speed < 0) ? 0 : target_right_speed;

    // 5. 速度环PI控制
    int32_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);

    // 6. 设置电机速度
    motor_set_speed(pwm_left, pwm_right);

    // 7. 获取编码器计数值
    int32_t count_left  = encoder_get_count_left();
    int32_t count_right = encoder_get_count_right();

    // 8. 计算轮子周长（厘米）
    float wheel_circum = 2.0f * (float)M_PI * WHEEL_RADIUS_CM;

    // 9. 计算本次采样的距离增量（使用两轮平均值）
    float delta_left  = count_left  * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    float delta_right = count_right * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);

    // 10. 累加距离（两轮平均值）
    track_accumulated_distance += (delta_left + delta_right) / 2.0f;

    // 11. 返回累计距离
    return track_accumulated_distance;
}

/**
 * @brief 重置循迹累计距离为零
 * @details 用于在开始新的移动任务前重置距离计数器
 */
void track_control_reset_distance(void) {
    track_accumulated_distance = 0.0f;
}

/**
 * @brief 梯形速度剖面：根据当前行驶距离返回平滑目标速度
 * @details 加速阶段: 0 → max_speed (距离 0 ~ ramp_dist)
 *          巡航阶段: max_speed 恒定 (距离 ramp_dist ~ total-ramp_dist)
 *          减速阶段: max_speed → 0 (距离 total-ramp_dist ~ total)
 *          到达后返回 0
 * @param current_dist 当前已行驶距离 (cm)
 * @param total_dist   总目标距离 (cm)
 * @param max_speed    巡航阶段速度 (RPM)
 * @param ramp_ratio   加减速区间占总距离的比例 (0 ~ 0.5)
 * @return 当前时刻的目标速度 (RPM)
 */
float speed_profile_trapezoid(float current_dist, float total_dist,
                               float max_speed, float ramp_ratio)
{
    float ramp_dist = total_dist * ramp_ratio;

    if (current_dist < 0.0f) return SMOOTH_MIN_START_SPEED;

    if (current_dist < ramp_dist) {
        /* ── 加速阶段：线性从 min_start 增长到 max_speed ── */
        float frac = current_dist / ramp_dist;
        return SMOOTH_MIN_START_SPEED + (max_speed - SMOOTH_MIN_START_SPEED) * frac;
    }
    else if (current_dist < total_dist - ramp_dist) {
        /* ── 巡航阶段：恒速 ── */
        return max_speed;
    }
    else if (current_dist < total_dist) {
        /* ── 减速阶段：线性从 max_speed 降到接近 0 ── */
        float remaining = total_dist - current_dist;
        float frac = remaining / ramp_dist;
        float speed = max_speed * frac;
        return (speed < SMOOTH_MIN_STOP_SPEED) ? 0.0f : speed;
    }
    else {
        /* ── 已到达目标 ── */
        return 0.0f;
    }
}

/**
 * @brief 带梯形速度剖面的循迹控制 + 距离累加
 * @details 与 track_control_get_distance 类似，但 base_speed 不是固定值，
 *          而是根据当前行驶距离通过梯形速度剖面动态生成，
 *          实现起步缓加速、结束前缓减速，降低钢球惯性晃动
 * @param total_dist  总目标距离 (cm)
 * @param max_speed   巡航阶段速度 (RPM)
 * @param ramp_ratio  加减速区间占比 (0~0.5)
 * @return 当前累计行驶距离 (cm)
 */
float track_control_smooth(float total_dist, float max_speed, float ramp_ratio)
{
    extern float current_speed_left;
    extern float current_speed_right;

    /* ── 1. 梯形剖面生成当前目标速度 ── */
    float base_speed = speed_profile_trapezoid(track_accumulated_distance,
                                                total_dist, max_speed, ramp_ratio);

    /* ── 2. 读取传感器并计算偏差 ── */
    float steer_err = track_get_error();
    steer_err_printf = steer_err;

    /* ── 3. PD转向控制 ── */
    float steer_adjust = track_pd_calc(&track_pid, steer_err);

    /* ── 4. 差速转向 ── */
    float target_left_speed  = base_speed + steer_adjust;
    float target_right_speed = base_speed - steer_adjust;

    /* ── 5. 防反转限幅 ── */
    if (target_left_speed  < 0) target_left_speed  = 0;
    if (target_right_speed < 0) target_right_speed = 0;

    /* ── 6. 速度环 PI 控制 ── */
    int32_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);

    /* ── 7. 设置电机 ── */
    motor_set_speed(pwm_left, pwm_right);

    /* ── 8. 距离累加 ── */
    int32_t count_left  = encoder_get_count_left();
    int32_t count_right = encoder_get_count_right();

    float wheel_circum = 2.0f * (float)M_PI * WHEEL_RADIUS_CM;

    float delta_left  = count_left  * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    float delta_right = count_right * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);

    track_accumulated_distance += (delta_left + delta_right) / 2.0f;

    return track_accumulated_distance;
}

/**
 * @brief 不对称速度剖面: 加速/减速距离独立控制
 * @details 加速阶段: 0 → accel_dist, 线性从 min_start 到 max_speed
 *          巡航阶段: accel_dist → total_dist-decel_dist, 恒速
 *          减速阶段: total_dist-decel_dist → total_dist, 线性降到 0
 * @param current_dist 当前已行驶距离 (cm)
 * @param total_dist   总目标距离 (cm)
 * @param max_speed    巡航阶段速度 (RPM)
 * @param accel_dist   加速区间长度 (cm)
 * @param decel_dist   减速区间长度 (cm)
 * @return 当前目标速度 (RPM)
 */
float speed_profile_asymmetric(float current_dist, float total_dist,
                                float max_speed, float accel_dist, float decel_dist)
{
    if (current_dist < 0.0f) return SMOOTH_MIN_START_SPEED;

    if (current_dist < accel_dist) {
        /* ── 加速阶段 ── */
        float frac = current_dist / accel_dist;
        return SMOOTH_MIN_START_SPEED + (max_speed - SMOOTH_MIN_START_SPEED) * frac;
    }
    else if (current_dist < total_dist - decel_dist) {
        /* ── 巡航阶段 ── */
        return max_speed;
    }
    else if (current_dist < total_dist) {
        /* ── 减速阶段 ── */
        float remaining = total_dist - current_dist;
        float frac = remaining / decel_dist;
        float speed = max_speed * frac;
        return (speed < SMOOTH_MIN_STOP_SPEED) ? 0.0f : speed;
    }
    else {
        return 0.0f;
    }
}

/**
 * @brief 不对称梯形剖面的循迹控制 + 距离累加
 * @details 加速和减速使用不同的距离, 适用于 Q4: 过线后从容减速
 */
float track_control_asymmetric(float total_dist, float max_speed,
                                float accel_dist, float decel_dist)
{
    extern float current_speed_left;
    extern float current_speed_right;

    float base_speed = speed_profile_asymmetric(track_accumulated_distance,
                                                 total_dist, max_speed,
                                                 accel_dist, decel_dist);

    float steer_err = track_get_error();
    steer_err_printf = steer_err;

    float steer_adjust = track_pd_calc(&track_pid, steer_err);

    float target_left_speed  = base_speed + steer_adjust;
    float target_right_speed = base_speed - steer_adjust;

    if (target_left_speed  < 0) target_left_speed  = 0;
    if (target_right_speed < 0) target_right_speed = 0;

    int32_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);

    motor_set_speed(pwm_left, pwm_right);

    int32_t count_left  = encoder_get_count_left();
    int32_t count_right = encoder_get_count_right();

    float wheel_circum = 2.0f * (float)M_PI * WHEEL_RADIUS_CM;

    float delta_left  = count_left  * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    float delta_right = count_right * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);

    track_accumulated_distance += (delta_left + delta_right) / 2.0f;

    return track_accumulated_distance;
}

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
    extern float current_speed_left;
    extern float current_speed_right;
    // ========== 1. 读取传感器并计算偏差 ==========
    float steer_err = track_get_error();
    steer_err_printf = steer_err; // 用于调试输出
    // ========== 2. 计算目标速度（转向环PD） ==========
    float target_left_speed;
    float target_right_speed;


    // 正常循迹：PD控制计算转向调整量
    float steer_adjust = track_pd_calc(&track_pid, steer_err);

    // 差速转向
    target_left_speed  = base_speed + steer_adjust;
    target_right_speed = base_speed - steer_adjust;

    // 防反转限幅
    target_left_speed  = (target_left_speed < 0)  ? 0 : target_left_speed;
    target_right_speed = (target_right_speed < 0) ? 0 : target_right_speed;


    // 计算电机PWM输出
    int32_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);

    // ========== 4. 设置电机速度 ==========
    motor_set_speed(pwm_left, pwm_right);
}

/**
 * @brief Q1专用循迹控制 (独立PID参数)
 */
void track_control_q1(float base_speed) {
    extern float current_speed_left;
    extern float current_speed_right;

    float steer_err = track_get_error();
    steer_err_printf = steer_err;

    float steer_adjust = track_pd_calc(&track_pid_q1, steer_err);

    float target_left_speed  = base_speed + steer_adjust;
    float target_right_speed = base_speed - steer_adjust;

    if (target_left_speed  < 0) target_left_speed  = 0;
    if (target_right_speed < 0) target_right_speed = 0;

    int32_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);

    motor_set_speed(pwm_left, pwm_right);
}

/**
 * @brief Q1专用循迹控制 + 距离累加 (独立PID参数)
 * @details 与 track_control_q1 功能相同，但额外累加编码器距离并返回
 * @param base_speed 基础速度值（RPM）
 * @return 当前累计行驶距离（厘米）
 */
float track_control_q1_get_distance(float base_speed) {
    extern float current_speed_left;
    extern float current_speed_right;

    float steer_err = track_get_error();
    steer_err_printf = steer_err;

    float steer_adjust = track_pd_calc(&track_pid_q1, steer_err);

    float target_left_speed  = base_speed + steer_adjust;
    float target_right_speed = base_speed - steer_adjust;

    if (target_left_speed  < 0) target_left_speed  = 0;
    if (target_right_speed < 0) target_right_speed = 0;

    int32_t pwm_left  = velocity_pi_calc(&Vpid_left,  current_speed_left,  target_left_speed);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_right_speed);

    motor_set_speed(pwm_left, pwm_right);

    /* ── 距离累加 ── */
    int32_t count_left  = encoder_get_count_left();
    int32_t count_right = encoder_get_count_right();

    float wheel_circum = 2.0f * (float)M_PI * WHEEL_RADIUS_CM;

    float delta_left  = count_left  * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);
    float delta_right = count_right * wheel_circum /
                        (ENCODER_LINE_COUNT * ENCODER_REDUCTION_RATIO * ENCODER_MODE_MULTIPLIER);

    track_accumulated_distance += (delta_left + delta_right) / 2.0f;

    return track_accumulated_distance;
}
