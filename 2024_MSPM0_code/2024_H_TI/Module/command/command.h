#ifndef __COMMAND_H_
#define __COMMAND_H_

#include "headfile.h"

uint8_t Command_Write(uint8_t *data, uint8_t length);

uint8_t Command_GetCommand(uint8_t *command);

uint8_t Command_GetResult(char *command,int *number);

#endif /* INC_COMMAND_H_ */
