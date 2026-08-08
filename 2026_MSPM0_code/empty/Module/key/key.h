/**
 * @file key.h
 * @brief 简单按键扫描：消抖 + 检测按下
 */

#ifndef __KEY_H__
#define __KEY_H__

#include "headfile.h"

/* 按键识别码 */
#define KEY_NONE     0x00
#define KEY_UP       0x01   /* K1: 上移 */
#define KEY_DOWN     0x02   /* K2: 下移 */
#define KEY_CONFIRM  0x04   /* K3: 确定 */
#define KEY_BACK     0x08   /* K4: 返回 */

void    key_init(void);
uint8_t key_scan(void);    /* 每10ms调用，消抖后返回按下的按键码，无按键返回KEY_NONE */

#endif
