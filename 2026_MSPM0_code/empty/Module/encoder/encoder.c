/**
 * @file Encoder.c
 * @brief 编码器模块实现
 * @details 实现编码器初始化、速度计算和PID控制相关功能
 */
#include "headfile.h"

volatile int32_t Encoder_Count_L=0;
volatile int32_t Encoder_Count_R=0;
extern char tx_buf[100];

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
 * @brief 浮点限幅函数（不截断小数）
 */
float limit_value_float(float value, float min, float max) {
    if(value > max) return max;
    if(value < min) return min;
    return value;
}

/**
 * @brief 编码器初始化函数
 * @details 初始化编码器定时器和采样定时器
 */
void encoder_init(void) {
     // 重置编码器计数器
    Encoder_Count_L = 0;
    Encoder_Count_R = 0;

    // 启用编码器相关中断
    NVIC_EnableIRQ(ENCODER_INT_IRQN);
}

/**
 * @brief 获取左轮编码器计数值
 * @return 左轮编码器的当前计数值
 */
int32_t encoder_get_count_left(void)  { return Encoder_Count_L; }

/**
 * @brief 获取右轮编码器计数值
 * @return 右轮编码器的当前计数值
 */
int32_t encoder_get_count_right(void) { return Encoder_Count_R; }

// 速度全局变量
float filt_velocity_left = 0.0f;      // 左轮滤波后速度
float last_filt_velocity_left = 0.0f; // 左轮上一次滤波速度
float filt_velocity_right = 0.0f;     // 右轮滤波后速度
float last_filt_velocity_right = 0.0f; // 右轮上一次滤波速度

/**
 * @brief 计算左轮速度
 * @return 左轮当前速度，单位：RPM
 */
float encoder_get_speed_left(void) {
    // 临界区：读-清零原子操作，防止编码器ISR在中间触发导致脉冲丢失
    int32_t left_count;
    __disable_irq();
    left_count = Encoder_Count_L;
    Encoder_Count_L = 0;
    __enable_irq();

    // 计算速度：(1000ms * 60s * 计数) / (编码器线数 * 减速比 * 倍频 * 采样时间)
    float velocity_left = (1000.0f * 60.0f * left_count) /
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
float encoder_get_speed_right(void) {
    // 临界区：读-清零原子操作，防止编码器ISR在中间触发导致脉冲丢失
    int32_t right_count;
    __disable_irq();
    right_count = Encoder_Count_R;
    Encoder_Count_R = 0;
    __enable_irq();

    // 计算速度：(1000ms * 60s * 计数) / (编码器线数 * 减速比 * 倍频 * 采样时间)
    float velocity_right = (1000.0f * 60.0f * right_count) /
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
int32_t velocity_pi_calc(PID *pid, float current_vel, float target_vel) {
    // 计算当前误差
    pid->err = target_vel - current_vel;
    // 计算误差积分
    pid->err_sum += pid->err;
    // 积分限幅，防止积分饱和
    pid->err_sum = limit_value_float(pid->err_sum, (float)INTEGRAL_MIN, (float)INTEGRAL_MAX);
    // 计算PI输出（+0.5f 四舍五入，避免截断死区）
    int32_t output = (int32_t)(pid->Kp * pid->err + pid->Ki * pid->err_sum + 0.5f);
    // 输出限幅
    return limit_value(output, PID_OUTPUT_MIN, PID_OUTPUT_MAX);
}

void  speed_control(float target_speed_left, float target_speed_right) {
    // 获取当前速度
    extern float current_speed_left;
    extern float current_speed_right;

    // 计算控制输出
    int32_t pwm_left  = velocity_pi_calc(&Vpid_left, current_speed_left, target_speed_left);
    int32_t pwm_right = velocity_pi_calc(&Vpid_right, current_speed_right, target_speed_right);

    // 设置电机速度
    motor_set_speed(pwm_left, pwm_right);
}



void GROUP1_IRQHandler(void)
{
	switch(DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
	{
		case DL_INTERRUPT_GROUP1_IIDX_GPIOB:

		/* ── 左轮编码器 A 相 ── */
		if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_LA_PIN))
		{
			uint8_t la = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_LA_PIN) != 0);
			uint8_t lb = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_LB_PIN) != 0);

			if(la == lb) Encoder_Count_L++;
			else         Encoder_Count_L--;

			DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_LA_PIN);
		}

		/* ── 左轮编码器 B 相 ── */
		if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_LB_PIN))
		{
			uint8_t la = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_LA_PIN) != 0);
			uint8_t lb = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_LB_PIN) != 0);

			if(la == lb) Encoder_Count_L--;
			else         Encoder_Count_L++;

			DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_LB_PIN);
		}

		/* ── 右轮编码器 A 相 ── */
		if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_RA_PIN))
		{
			uint8_t ra = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_RA_PIN) != 0);
			uint8_t rb = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_RB_PIN) != 0);

			if(ra == rb) Encoder_Count_R--;
			else         Encoder_Count_R++;

			DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_RA_PIN);
		}

		/* ── 右轮编码器 B 相 ── */
		if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_RB_PIN))
		{
			uint8_t ra = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_RA_PIN) != 0);
			uint8_t rb = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_RB_PIN) != 0);

			if(ra == rb) Encoder_Count_R++;
			else         Encoder_Count_R--;

			DL_GPIO_clearInterruptStatus(ENCODER_PORT, ENCODER_RB_PIN);
		}

		break;
	}
}
