#include "command.h"
#include <stdio.h>
#include <stdint.h>

// 指令的最小长度
#define COMMAND_MIN_LENGTH 4

// 循环缓冲区大小
#define BUFFER_SIZE 128
// 循环缓冲区
uint8_t buffer[BUFFER_SIZE];
// 循环缓冲区读索引
uint8_t readIndex = 0;
// 循环缓冲区写索引
uint8_t writeIndex = 0;

/**
* @brief 增加读索引
* @param length 要增加的长度
*/
void Command_AddReadIndex(uint8_t length) {
    readIndex += length;
    readIndex %= BUFFER_SIZE;
}

/**
* @brief 读取第i位数据 超过缓存区长度自动循环
* @param i 要读取的数据索引
*/

uint8_t Command_Read(uint8_t i) {
    uint8_t index = i % BUFFER_SIZE;
    return buffer[index];
}

/**
* @brief 计算未处理的数据长度
* @return 未处理的数据长度
* @retval 0 缓冲区为空
* @retval 1~BUFFER_SIZE-1 未处理的数据长度
* @retval BUFFER_SIZE 缓冲区已满
*/
//uint8_t Command_GetLength() {
//  // 读索引等于写索引时，缓冲区为空
//  if (readIndex == writeIndex) {
//    return 0;
//  }
//  // 如果缓冲区已满,返回BUFFER_SIZE
//  if (writeIndex + 1 == readIndex || (writeIndex == BUFFER_SIZE - 1 && readIndex == 0)) {
//    return BUFFER_SIZE;
//  }
//  // 如果缓冲区未满,返回未处理的数据长度
//  if (readIndex <= writeIndex) {
//    return writeIndex - readIndex;
//  } else {
//    return BUFFER_SIZE - readIndex + writeIndex;
//  }
//}

uint8_t Command_GetLength() {
    return (writeIndex + BUFFER_SIZE - readIndex) % BUFFER_SIZE;
}


/**
* @brief 计算缓冲区剩余空间
* @return 剩余空间
* @retval 0 缓冲区已满
* @retval 1~BUFFER_SIZE-1 剩余空间
* @retval BUFFER_SIZE 缓冲区为空
*/
uint8_t Command_GetRemain() {
    return BUFFER_SIZE - Command_GetLength();
}

/**
* @brief 向缓冲区写入数据
* @param data 要写入的数据指针
* @param length 要写入的数据长度
* @return 写入的数据长度
*/
uint8_t Command_Write(uint8_t *data, uint8_t length) {
    // 如果缓冲区不足 则不写入数据 返回0
    if (Command_GetRemain() < length) {
        return 0;
    }
    // 使用memcpy函数将数据写入缓冲区
    if (writeIndex + length < BUFFER_SIZE) {
        memcpy(buffer + writeIndex, data, length);
        writeIndex += length;
    } else {
        uint8_t firstLength = BUFFER_SIZE - writeIndex;
        memcpy(buffer + writeIndex, data, firstLength);
        memcpy(buffer, data + firstLength, length - firstLength);
        writeIndex = length - firstLength;
    }
    return length;
}

/**
* @brief 尝试获取一条指令
* @param command 指令存放指针
* @return 获取的指令长度
* @retval 0 没有获取到指令
*/
uint8_t Command_GetCommand(uint8_t *command) {
    while (1) {
        if (Command_GetLength() < COMMAND_MIN_LENGTH) {
            return 0;
        }
        // 找包头 'A'
        if (Command_Read(readIndex) != 'A') {
            Command_AddReadIndex(1);
            continue;
        }
        // 读载荷长度（ASCII 数字 1~9）
        uint8_t payload_len = Command_Read(readIndex + 1) - '0';
        if (payload_len < 1 || payload_len > 9) {
            Command_AddReadIndex(1);
            continue;
        }
        // 总长 = A(1) + 长度数字(1) + 载荷 + Z(1)
        uint8_t total_len = payload_len + 3;
        if (Command_GetLength() < total_len) {
            return 0;
        }
        // 检查包尾 'Z'
        if (Command_Read(readIndex + total_len - 1) != 'Z') {
            Command_AddReadIndex(1);
            continue;
        }
        // 提取整包
        for (uint8_t i = 0; i < total_len; i++) {
            command[i] = Command_Read(readIndex + i);
        }
        Command_AddReadIndex(total_len);
        return total_len;
    }
}

uint8_t commands[10];
uint8_t commandLength = 0;

uint8_t Command_GetResult(char *command, int *number) {
    commandLength = Command_GetCommand(commands);
    if (commandLength != 0) {
        *command = commands[2];
        *number = 0;
        for (uint8_t i = 3; i < commandLength - 1; i++) {
            *number = *number * 10 + (commands[i] - '0');
        }
        return 1;
    }
    return 0;
}