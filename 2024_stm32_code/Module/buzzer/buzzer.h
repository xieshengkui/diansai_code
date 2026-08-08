/**
 * @file buzzer.h
 * @brief 蜂鸣器驱动头文件
 * 
 * 本文件提供蜂鸣器的控制接口，包括蜂鸣器的开启、关闭和定时控制功能。
 */

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "headfile.h"

/**
 * @brief 蜂鸣器持续时间计数器（毫秒）
 * 
 * 用于控制蜂鸣器响铃时长，在定时中断中递减，减至0时自动关闭蜂鸣器。
 */
extern volatile uint16_t buzzer_time_ms;

/**
 * @brief 开启蜂鸣器
 * 
 * 将蜂鸣器引脚设置为低电平，使蜂鸣器发声。
 */
void buzzer_on(void);

/**
 * @brief 关闭蜂鸣器
 * 
 * 将蜂鸣器引脚设置为高电平，停止蜂鸣器发声。
 */
void buzzer_off(void);

/**
 * @brief 蜂鸣器定时控制函数
 * 
 * 在定时中断中调用此函数，根据buzzer_time_ms的值控制蜂鸣器的开关。
 * 当buzzer_time_ms > 0时开启蜂鸣器并递减计数器，否则关闭蜂鸣器。
 */
void buzzer_control(void);

#endif