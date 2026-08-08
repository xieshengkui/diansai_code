/**
 * @file buzzer.c
 * @brief 蜂鸣器驱动实现文件
 * 
 * 实现蜂鸣器的硬件控制逻辑，基于STM32 HAL库操作GPIO引脚。
 */

#include "headfile.h"
#include "main.h"
#include <stdint.h>

/**
 * @brief 开启蜂鸣器
 * 
 * 将蜂鸣器控制引脚置低电平，触发蜂鸣器发声。
 * 使用HAL库GPIO写函数操作硬件引脚。
 */
void buzzer_on(void) {
    HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭蜂鸣器
 * 
 * 将蜂鸣器控制引脚置高电平，停止蜂鸣器发声。
 */
void buzzer_off(void) {
    HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, GPIO_PIN_SET);
}

/**
 * @brief 蜂鸣器持续时间计数器
 * 
 * 单位：毫秒。设置非零值后，蜂鸣器会持续响铃相应时长。
 * 需在10ms定时中断中调用buzzer_control()进行递减。
 */
volatile uint16_t buzzer_time_ms = 0;

/**
 * @brief 蜂鸣器定时控制函数
 * 
 * 在10ms定时中断中调用，实现蜂鸣器的定时响铃功能。
 * - 当buzzer_time_ms > 0时：开启蜂鸣器，计数器减10ms
 * - 当buzzer_time_ms <= 0时：关闭蜂鸣器
 */
void buzzer_control(void) {
    if(buzzer_time_ms > 0) {
        buzzer_on();
        buzzer_time_ms -= 10;  // 每次调用减少10ms（假设10ms调用一次）
    } else {
        buzzer_off();
    }
}