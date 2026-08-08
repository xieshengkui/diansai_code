/**
 * @file key.c
 * @brief 简单按键扫描：消抖 + 检测按下
 * @details 每10ms调用 key_scan()，返回当前按下的按键码。
 *          按下=低电平，释放=高电平（上拉输入）。
 */

#include "headfile.h"

#define DEBOUNCE_MS  30        /* 消抖时间(ms) */
#define SCAN_PERIOD  10        /* 扫描周期(ms) */
#define DEBOUNCE_CNT (DEBOUNCE_MS / SCAN_PERIOD)  /* 消抖计数阈值 = 3 */

static struct {
    uint8_t cnt;        /* 当前连续按下的扫描次数 */
    uint8_t confirmed;  /* 已确认按下（防重复触发） */
} keys[4];

/* 4键对应的key_code, 按K1~K4顺序 */
static const uint8_t key_codes[4] = { KEY_UP, KEY_DOWN, KEY_CONFIRM, KEY_BACK };

void key_init(void) {
    for (int i = 0; i < 4; i++) {
        keys[i].cnt       = 0;
        keys[i].confirmed = 0;
    }
}

/**
 * @brief 扫描按键，每10ms调用
 * @return 消抖后确认按下的按键码，无按键返回 KEY_NONE
 * @note   按住不松手只触发一次，松手后才能再次触发
 */
uint8_t key_scan(void) {
    /* 一次性读取4个键: 按下=0(LOW) */
    uint8_t raw[4];
    raw[0] = (DL_GPIO_readPins(KEY_K1_PORT, KEY_K1_PIN) == 0);
    raw[1] = (DL_GPIO_readPins(KEY_K2_PORT, KEY_K2_PIN) == 0);
    raw[2] = (DL_GPIO_readPins(KEY_K3_PORT, KEY_K3_PIN) == 0);
    raw[3] = (DL_GPIO_readPins(KEY_K4_PORT, KEY_K4_PIN) == 0);

    uint8_t result = KEY_NONE;

    for (int i = 0; i < 4; i++) {
        if (raw[i]) {
            /* 按下：累加计数 */
            if (keys[i].cnt < 255) keys[i].cnt++;

            /* 达到消抖阈值且未确认过 → 触发 */
            if (keys[i].cnt >= DEBOUNCE_CNT && !keys[i].confirmed) {
                keys[i].confirmed = 1;
                result = key_codes[i];
            }
        } else {
            /* 释放：重置计数和确认标志 */
            keys[i].cnt       = 0;
            keys[i].confirmed = 0;
        }
    }

    return result;
}
