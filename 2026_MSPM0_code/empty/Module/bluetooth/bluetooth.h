#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "headfile.h"

void bluetooth_init(void);
void bluetooth_write(uint8_t *data, uint16_t size);
void bluetooth_send_command(char cmd, uint32_t number);

#endif
