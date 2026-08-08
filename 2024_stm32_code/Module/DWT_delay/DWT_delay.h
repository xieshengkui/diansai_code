// d:\Desktop\Cmake\text\Module\DWT_delay\DWT_delay.h
#ifndef __DWT_DELAY_H
#define __DWT_DELAY_H

#include "stm32f4xx.h"

/**
 * @defgroup DWT_Delay DWT延时函数模块
 * @brief 使用DWT（数据观察点和跟踪）实现高精度微秒级延时
 * 
 * @details DWT是Cortex-M处理器中的调试组件，主要功能包括：
 * - CYCCNT：周期计数器，用于计数CPU时钟周期
 * - 数据观察点，用于监控内存访问
 * - 指令跟踪功能
 * 
 * 本模块使用CYCCNT寄存器实现高精度微秒延时，相比传统延时方法具有以下优势：
 * - 最高精度（系统时钟周期级别）
 * - 不占用定时器资源
 * - 可在中断中使用
 * - 不受编译器优化影响
 * 
 * @{
 */

/**
 * @brief 初始化DWT周期计数器
 * 
 * @details 使能DWT单元并启动周期计数器。
 * 在使用任何延时函数之前必须调用此函数一次。
 */
void DWT_Init(void);

/**
 * @brief DWT微秒级延时
 * 
 * @param us 延时时间（单位：微秒）
 * @note 需要正确定义SystemCoreClock
 */
void delay_us(uint32_t us);

/**
 * @brief DWT毫秒级延时
 * 
 * @param ms 延时时间（单位：毫秒）
 */
void delay_ms(uint32_t ms);

/**
 * @brief 获取当前周期计数器值
 * 
 * @return DWT->CYCCNT寄存器的当前值
 */
static inline uint32_t DWT_GetCycles(void)
{
    return DWT->CYCCNT;
}

/** @} */

#endif /* __DWT_DELAY_H */