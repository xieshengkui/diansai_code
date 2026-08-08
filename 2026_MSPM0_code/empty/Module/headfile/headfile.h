#ifndef __HEADFILE_H
#define __HEADFILE_H

#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "math.h"

#define PI  3.14159265f   // 统一圆周率定义

extern volatile uint32_t sys_tick_ms;
extern volatile uint32_t lap_start_tick;
extern volatile uint32_t lap_stop_tick;
extern uint32_t last_k230_time;
extern float pos_error;

#include "I2C_communication.h"
#include "IMU.h"
#include "motor.h"
#include "encoder.h"
#include "position.h"
#include "track.h"
#include "command.h"
#include "delay.h"
#include "turn.h"
#include "bluetooth.h"
#include "K230.h"
#include "question_task.h"
#include "key.h"
#include "menu.h"
#include "oled.h"
#include "oledfont.h"
#include "bmp.h"
#include "stepper.h"
extern float target_speed;

#endif