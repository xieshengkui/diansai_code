// d:\Desktop\Cmake\text\Module\DWT_delay\DWT_delay.c
#include "DWT_delay.h"

/**
 * @defgroup DWT_Registers DWT寄存器定义
 * @brief DWT单元的内存映射寄存器定义
 * @{
 */

/** DWT控制寄存器 */
#define DWT_CTRL        (*((volatile uint32_t *)0xE0001000))

/** DWT周期计数器寄存器 */
#define DWT_CYCCNT      (*((volatile uint32_t *)0xE0001004))

/** DWT CYCCNT使能位掩码 */
#define DWT_CTRL_CYCCNTENA    (1 << 0)

/** CoreDebug DEMCR寄存器 */
#define CoreDebug_DEMCR      (*((volatile uint32_t *)0xE000EDFC))

/** CoreDebug TRCENA使能位掩码 */
#define CoreDebug_DEMCR_TRCENA    (1 << 24)

/** @} */

/**
 * @brief 初始化DWT周期计数器
 * 
 * @details DWT（数据观察点和跟踪）单元是Cortex-M调试基础设施的一部分。
 * CYCCNT是一个32位自由运行计数器，每个CPU时钟周期递增一次。
 * 
 * 初始化步骤：
 * 1. 通过CoreDebug->DEMCR寄存器使能DWT单元
 * 2. 将CYCCNT计数器重置为0
 * 3. 通过DWT->CTRL寄存器使能CYCCNT计数器
 * 
 * @note 此函数应在系统时钟初始化完成后调用。
 */
void DWT_Init(void)
{
    // 通过设置CoreDebug->DEMCR的TRCENA位使能DWT单元
    if (!(CoreDebug_DEMCR & CoreDebug_DEMCR_TRCENA)) {
        CoreDebug_DEMCR |= CoreDebug_DEMCR_TRCENA;
    }
    
    // 重置周期计数器
    DWT_CYCCNT = 0;
    
    // 使能周期计数器
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}

/**
 * @brief DWT微秒级延时
 * 
 * @details 根据指定的延时时间计算所需的CPU周期数，然后等待计数器达到该值。
 * 
 * 使用公式：cycles = SystemCoreClock / 1000000 * us
 * 其中SystemCoreClock是CPU时钟频率（单位：Hz）。
 * 
 * @param us 延时时间（单位：微秒）
 * 
 * @note SystemCoreClock必须在项目中正确定义（通常由HAL库在系统初始化时设置）
 */
void delay_us(uint32_t us)
{
    uint32_t start_cycles = DWT_CYCCNT;
    uint32_t cycles_needed = (SystemCoreClock / 1000000U) * us;
    
    // 处理CYCCNT溢出情况（CYCCNT是32位寄存器，在0xFFFFFFFF处溢出）
    if (start_cycles + cycles_needed < start_cycles) {
        // 等待计数器溢出
        while (DWT_CYCCNT > start_cycles);
        cycles_needed -= (0xFFFFFFFF - start_cycles + 1);
    }
    
    // 等待直到经过了所需的周期数
    while ((DWT_CYCCNT - start_cycles) < cycles_needed);
}

/**
 * @brief DWT毫秒级延时
 * 
 * @details 通过循环调用delay_us实现毫秒级延时。
 * 对于大延时，这种方式比单次调用delay_us更高效，因为它更优雅地处理了CYCCNT溢出问题。
 * 
 * @param ms 延时时间（单位：毫秒）
 */
void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_us(1000);
    }
}