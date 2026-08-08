/**
 * @file K230.h
 * @brief K230 串口通信模块
 * @details 接收 K230 摄像头通过 UART3 (115200bps) 发来的数据，
 *          通过 DMA RX + 环形缓冲区实现高效接收。
 */

#ifndef __K230_H
#define __K230_H

#include "headfile.h"

/* ── 接收环形缓冲区 ── */
#define K230_RX_BUF_SIZE  256

void k230_init(void);
uint8_t k230_write(uint8_t *data, uint16_t size);
uint16_t k230_available(void);           /* 缓冲区可读字节数 */
uint8_t k230_read_byte(uint8_t *byte);   /* 读单字节, 返回1成功0无数据 */
uint16_t k230_read(uint8_t *buf, uint16_t len); /* 批量读取 */

#endif
