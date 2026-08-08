#ifndef __MENU_H__
#define __MENU_H__

#include "headfile.h"

/* ── 菜单状态 ── */
typedef enum {
    MENU_SELECTING = 0,  /* 浏览选题菜单 */
    MENU_RUNNING,        /* 题目执行中 */
    MENU_STOPPED         /* 任务已停止 */
} menu_state_t;

extern menu_state_t menu_state;
extern uint8_t      menu_selected_index;  /* 0=T1, 1=T2, 2=T3, 3=T4, 4=T5 */


void menu_init(void);
void menu_key_process(void);      /* 10ms 按键扫描 + 状态切换，task[3] */
void menu_display_process(void);  /* 200ms 显示刷新，task[2] */
void menu_display(void);

#endif
