#ifndef __COMMAND_H_
#define __COMMAND_H_

#include "headfile.h"
#include <stdint.h>  // 保证uint8_t/int32_t标准类型可用，若headfile.h已包含可删除

/**
 * @brief  向环形缓冲区写入数据
 * @param  data   数据指针
 * @param  length 写入长度
 * @return 实际写入长度
 */
uint8_t Command_Write(uint8_t *data, uint8_t length);

/**
 * @brief  从缓冲区提取一条完整报文
 * @param  command 报文存放指针
 * @return 报文总长度，0=无有效报文
 */
uint8_t Command_GetCommand(uint8_t *command);

/**
 * @brief  解析报文，输出命令字符、数值
 *         格式: S + len + cmd + ASCII数字 + Z
 *         cmd大写=正数, 小写=负数 (如'X'=+20, 'x'=-20)
 * @param  cmd_char  输出：命令字符(已转大写)
 * @param  number    输出：解析后的数值(带符号)
 * @return 1=解析成功，0=无有效报文
 */
uint8_t Command_GetResult(char *cmd_char, int32_t *number);

#endif /* INC_COMMAND_H_ */