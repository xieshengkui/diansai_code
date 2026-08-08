/**
 * @file bluetooth.c
 * @brief 蓝牙调试模块 (仅DMA发送)
 * @details 保留 UART TX DMA 用于调试数据发送，已移除指令接收功能
 */

#include "bluetooth.h"

/**
 * @brief 蓝牙DMA发送初始化
 */
void bluetooth_init(void)
{
    DL_DMA_enableChannel(DMA, bluetooth_DMA_CHAN_ID);
}

/**
 * @brief 通过DMA发送数据 (阻塞式)
 * @param data 数据指针
 * @param size 数据长度
 */
void bluetooth_write(uint8_t *data, uint16_t size)
{
    DL_DMA_setSrcAddr(DMA, bluetooth_DMA_CHAN_ID, (uint32_t)data);
    DL_DMA_setDestAddr(DMA, bluetooth_DMA_CHAN_ID, (uint32_t)(&BT_INST->TXDATA));
    DL_DMA_setTransferSize(DMA, bluetooth_DMA_CHAN_ID, size);
    DL_DMA_enableChannel(DMA, bluetooth_DMA_CHAN_ID);
}

/**
 * @brief 按协议格式发送调试指令
 * @details 格式: A + len + cmd + data + Z
 *          例: bluetooth_send_command('V', 50) → "A2V50Z"
 * @param cmd  命令字符
 * @param number 数值
 */
void bluetooth_send_command(char cmd, uint32_t number)
{
    char buf[12];
    uint8_t data_len;

    if (number == 0) {
        data_len = 1;
    } else {
        data_len = 0;
        uint32_t n = number;
        while (n > 0) {
            data_len++;
            n /= 10;
        }
    }

    uint8_t payload_len = 1 + data_len;

    buf[0] = 'A';
    buf[1] = '0' + payload_len;
    buf[2] = cmd;

    uint8_t pos = 3 + data_len;
    buf[pos] = 'Z';

    uint32_t n = number;
    for (uint8_t i = 0; i < data_len; i++) {
        buf[--pos] = '0' + (n % 10);
        n /= 10;
    }

    bluetooth_write((uint8_t *)buf, 3 + data_len + 1);
}
