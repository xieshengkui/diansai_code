/**
 * @file K230.c
 * @brief K230 串口通信模块实现
 * @details UART3 + DMA RX 环形缓冲区接收 K230 摄像头数据。
 *          中断接收 → 环形缓冲 → 上层轮询读取。
 */

#include "K230.h"

/* ── 环形接收缓冲区 ── */
static uint8_t  rx_buf[K230_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;  /* 写指针 (ISR) */
static volatile uint16_t rx_tail = 0;  /* 读指针 (主循环) */
static uint8_t  rx_byte;               /* DMA/ISR 单字节接收暂存 */

/**
 * @brief K230 初始化
 * @details 使能 DMA 通道、UART 中断，启用接收
 */
void k230_init(void)
{
    DL_DMA_enableChannel(DMA, K230_DMA_CHAN_ID);

    NVIC_ClearPendingIRQ(K230_INST_INT_IRQN);
    NVIC_EnableIRQ(K230_INST_INT_IRQN);
    DL_UART_enableInterrupt(K230_INST, DL_UART_INTERRUPT_RX);
}

/**
 * @brief 通过 UART 发送数据到 K230 (阻塞式)
 * @param data 数据指针
 * @param size 数据长度
 */
uint8_t k230_write(uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        while (DL_UART_isBusy(K230_INST) == true);
        DL_UART_Main_transmitData(K230_INST, data[i]);
    }
    return size;
}

/**
 * @brief 返回环形缓冲区中可读的字节数
 */
uint16_t k230_available(void)
{
    return (rx_head + K230_RX_BUF_SIZE - rx_tail) % K230_RX_BUF_SIZE;
}

/**
 * @brief 从环形缓冲区读取单字节
 * @param byte 输出：读取的字节
 * @return 1=成功, 0=缓冲区空
 */
uint8_t k230_read_byte(uint8_t *byte)
{
    if (rx_head == rx_tail) return 0;
    *byte = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % K230_RX_BUF_SIZE;
    return 1;
}

/**
 * @brief 从环形缓冲区批量读取
 * @param buf 目标缓冲区
 * @param len 最大读取长度
 * @return 实际读取的字节数
 */
uint16_t k230_read(uint8_t *buf, uint16_t len)
{
    uint16_t count = 0;
    while (count < len && rx_head != rx_tail) {
        buf[count++] = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % K230_RX_BUF_SIZE;
    }
    return count;
}

/**
 * @brief K230 UART3 中断处理
 * @details RX 中断将接收字节写入环形缓冲区，同时喂给 Command 解析器
 */
void K230_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(K230_INST))
    {
        case DL_UART_IIDX_RX:
            rx_byte = DL_UART_Main_receiveData(K230_INST);
            /* 写入本地环形缓冲 */
            uint16_t next = (rx_head + 1) % K230_RX_BUF_SIZE;
            if (next != rx_tail) {
                rx_buf[rx_head] = rx_byte;
                rx_head = next;
            }
            /* 同步喂给指令解析器 */
            Command_Write(&rx_byte, 1);
            break;

        default:
            break;
    }
}
