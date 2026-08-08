/**
 * @file motor.c
 * @brief 电机驱动模块实现
 * @details 实现电机初始化和速度控制相关功能
 */
#include "headfile.h"

/**
 * @brief 电机驱动初始化函数
 * @details 初始化电机的PWM输出定时器，启动左右电机的PWM输出通道
 */
void motor_init() {
    // 启动TIM2通道1（左轮PWM）和TIM3通道1（右轮PWM）的PWM输出
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
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
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
        TIM2->CCR1 = CCR_num;
    } else if(CCR_num < 0) {
        // 反转：设置方向引脚，写入PWM值（取绝对值）
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
        TIM2->CCR1 = -CCR_num;
    } else {
        // 停止：PWM设为0，方向引脚全部拉低，刹车
        TIM2->CCR1 = 0;
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
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
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
        TIM3->CCR1 = CCR_num;
    } else if(CCR_num < 0) {
        // 反转：设置方向引脚，写入PWM值（取绝对值）
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
        TIM3->CCR1 = -CCR_num;
    } else {
        // 停止：PWM设为0，方向引脚全部拉低，刹车
        TIM3->CCR1 = 0;
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
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