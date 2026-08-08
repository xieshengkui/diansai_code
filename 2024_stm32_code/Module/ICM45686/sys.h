#ifndef _SYS_H 
#define _SYS_H 

#include "stm32f4xx.h" 

// 定义常用数据类型
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

// 位带操作宏
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

// GPIO地址映射（仅保留需要的端口）
#define GPIOB_ODR_Addr    (GPIOB_BASE+20)
#define GPIOB_IDR_Addr    (GPIOB_BASE+16)

// IO口操作宏（仅保留需要的端口）
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)

#endif