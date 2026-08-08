/**
 * @file motor.c
 * @brief 电机驱动模块实现
 * @details 实现电机初始化和速度控制相关功能
 */
#include "headfile.h"

/**
 * @brief 电机驱动初始化函数
 * @details 初始化电机的PWM输出定时器，启动左右电机的PWM输出通道
 * 使用TIMER_0的两个比较通道：COMP0（左轮）和COMP1（右轮）
 */
void motor_init() {
    // 启动TIMER_0的两个比较通道PWM输出
    DL_Timer_startCounter(PWMA_INST);
    DL_Timer_startCounter(PWMB_INST);
}

/**
 * @brief 设置左轮电机的运行速度与方向
 * @param CCR_num 速度控制值：
 *        - 正数：正转，值为PWM比较寄存器值，决定占空比
 *        - 负数：反转，绝对值为PWM比较寄存器值
 *        - 0：刹车停止
 *        取值范围：[-1000, 1000]，与PID输出限幅对齐
 */
void motor_set_left_speed(int32_t CCR_num) {
    if(CCR_num > 0) {
        // 正转：设置方向引脚，写入PWM值
        DL_GPIO_clearPins(TB6612_PORT, TB6612_AIN1_PIN);
        DL_GPIO_setPins(TB6612_PORT, TB6612_AIN2_PIN);   
        DL_TimerG_setCaptureCompareValue(PWMA_INST, CCR_num, GPIO_PWMA_C0_IDX);
    } else if(CCR_num < 0) {
        // 反转：设置方向引脚，写入PWM值（取绝对值）
        DL_GPIO_setPins(TB6612_PORT, TB6612_AIN1_PIN);     // AIN1 = HIGH
        DL_GPIO_clearPins(TB6612_PORT, TB6612_AIN2_PIN);   // AIN2 = LOW
        DL_TimerG_setCaptureCompareValue(PWMA_INST, -CCR_num, GPIO_PWMA_C0_IDX);
    } else {
        // 停止：PWM设为0，方向引脚全部拉低，刹车
        DL_TimerG_setCaptureCompareValue(PWMA_INST, 0, GPIO_PWMA_C0_IDX);
        DL_GPIO_clearPins(TB6612_PORT, TB6612_AIN1_PIN);   // AIN1 = LOW
        DL_GPIO_clearPins(TB6612_PORT, TB6612_AIN2_PIN);   // AIN2 = LOW
    }
}

/**
 * @brief 设置右轮电机的运行速度与方向
 * @param CCR_num 速度控制值：
 *        - 正数：正转，值为PWM比较寄存器值，决定占空比
 *        - 负数：反转，绝对值为PWM比较寄存器值
 *        - 0：刹车停止
 *        取值范围：[-1000, 1000]，与PID输出限幅对齐
 */
void motor_set_right_speed(int32_t CCR_num) {
    if(CCR_num > 0) {
        // 正转：设置方向引脚，写入PWM值
        DL_GPIO_setPins(TB6612_PORT, TB6612_BIN1_PIN);     // BIN1 = HIGH
        DL_GPIO_clearPins(TB6612_PORT, TB6612_BIN2_PIN);   // BIN2 = LOW
        DL_TimerG_setCaptureCompareValue(PWMB_INST, CCR_num, GPIO_PWMB_C1_IDX);
    } else if(CCR_num < 0) {
        // 反转：设置方向引脚，写入PWM值（取绝对值）
        DL_GPIO_clearPins(TB6612_PORT, TB6612_BIN1_PIN);   // BIN1 = LOW
        DL_GPIO_setPins(TB6612_PORT, TB6612_BIN2_PIN);     // BIN2 = HIGH
        DL_TimerG_setCaptureCompareValue(PWMB_INST, -CCR_num, GPIO_PWMB_C1_IDX);
    } else {
        // 停止：PWM设为0，方向引脚全部拉低，刹车
        DL_TimerG_setCaptureCompareValue(PWMB_INST, 0, GPIO_PWMB_C1_IDX);
        DL_GPIO_clearPins(TB6612_PORT, TB6612_BIN1_PIN);   // BIN1 = LOW
        DL_GPIO_clearPins(TB6612_PORT, TB6612_BIN2_PIN);   // BIN2 = LOW
    }
}

/**
 * @brief 同时设置左右轮电机的运行速度
 * @param left_num 左轮速度控制值，同motor_set_left_speed的参数
 * @param right_num 右轮速度控制值，同motor_set_right_speed的参数
 */
void motor_set_speed(int32_t left_num, int32_t right_num) {
    motor_set_left_speed(left_num);
    motor_set_right_speed(right_num);
}