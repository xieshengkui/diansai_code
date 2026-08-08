/**
 * @file menu.c
 * @brief OLED菜单系统：5任务选题 / 运行中 / 已停止 三状态
 * @details 字体大小16 (8x16), 4行显示; KEY1=上移, KEY2=下移, KEY3=确定, KEY4=返回
 *          5项菜单自动滚动; 显示仅在按键/状态切换时刷新
 */

#include "menu.h"
#include "key.h"
#include "question_task.h"
#include <stdio.h>

extern uint8_t flag1;  /* Q2 阶段标志, 0=去+50mm, 1=去-50mm */
extern uint8_t flag2;

/* ── 全局状态 ── */
menu_state_t menu_state          = MENU_SELECTING;
uint8_t      menu_selected_index = 0;  /* 0~4 对应 Task1~5 */

/* ── 显示脏标记 ── */
static uint8_t display_dirty = 1;

/* ── 五个任务的标签 (≤16字符适配128px宽) ── */
static const char *task_labels[] = {
    "1.Lap 20s",
    "2.Ball 5s",
    "3.AB 8s",
    "4.Lap+O",
    "5.Lap+X"
};

/* ── 内部函数声明 ── */
static void draw_selecting(void);
static void draw_running(void);
static void draw_stopped(void);
static void format_time(uint32_t ms, char *buf, uint8_t size);

/* ═══════════════════════════════════════════════════════════════════
 * 公开API
 * ═══════════════════════════════════════════════════════════════════ */

void menu_init(void) {
    menu_state          = MENU_SELECTING;
    menu_selected_index = 0;
    display_dirty       = 1;
}

/* ═══════════════════════════════════════════════════════════════════
 * menu_key_process  — 10ms 按键扫描 + 状态切换 (task[3])
 * ═══════════════════════════════════════════════════════════════════ */
void menu_key_process(void) {
    uint8_t key = key_scan();

    switch (menu_state) {

        case MENU_SELECTING:
            switch (key) {
            case KEY_UP:
                if (menu_selected_index > 0)
                    menu_selected_index--;
                else
                    menu_selected_index = 4;
                display_dirty = 1;
                break;

            case KEY_DOWN:
                if (menu_selected_index < 4)
                    menu_selected_index++;
                else
                    menu_selected_index = 0;
                display_dirty = 1;
                break;

            case KEY_CONFIRM: {
                uint8_t q_states[] = { Q1_STATE, Q2_STATE, Q3_STATE, Q4_STATE, Q5_STATE };
                STATE_MACHINE.Main_State = q_states[menu_selected_index];

                if (menu_selected_index == 1)
                {
                    q2_first_flag = 0;
                    flag1 = 0;
                    ball_pid_set_static();   /* Q2 折返: 小车静止PID套 */
                }
                if (menu_selected_index == 2){
                    q3_first_flag = 0;
                    ball_pid_set_q3();
                }
                if (menu_selected_index == 3){
                    q4_first_flag = 0;
                    ball_pid_set_q4();
                }
                if (menu_selected_index == 4) {
                    q5_first_flag = 0;
                    stepper_goto_horizontal();   /* Q5第1次: 水管归零, 不调球 */
                }
                
                menu_state     = MENU_RUNNING;
                display_dirty  = 1;
                break;
            }

            default:
                break;
            }
            break;

        case MENU_RUNNING:
            if (key == KEY_BACK) {
                STATE_MACHINE.Main_State = STOP_STATE;
                speed_control(0.0f, 0.0f);
                stepper_goto_horizontal();
                ball_pid_set_static();          /* 回到菜单用PID集2 */

                STATE_MACHINE.Q1_State = Q1_State_1;
                STATE_MACHINE.Q2_State = Q2_State_1;
                STATE_MACHINE.Q3_State = Q3_State_1;
                STATE_MACHINE.Q4_State = Q4_State_1;
                STATE_MACHINE.Q5_State = Q5_State_1;
                q4_lap_count = 0;

                menu_state     = MENU_STOPPED;
                display_dirty  = 1;
            }
            /* Q5第2次KEY3: 采样并开始跑 */
            if (key == KEY_CONFIRM
                && STATE_MACHINE.Main_State == Q5_STATE
                && STATE_MACHINE.Q5_State == Q5_State_1) {
                STATE_MACHINE.Q5_State = Q5_State_2;
            }
            break;

        case MENU_STOPPED:
            flag2 = 0;
            switch (key) {
            case KEY_BACK:
            case KEY_CONFIRM:
                menu_state    = MENU_SELECTING;
                display_dirty = 1;
                ball_pid_set_static();      /* 菜单界面用PID集2 */
                break;
            default:
                break;
            }
            break;
    }

    /* 按键触发的状态变化立即刷新屏幕 */
    if (display_dirty) {
        menu_display();
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * menu_display_process — 200ms 显示刷新 (task[2])
 * ═══════════════════════════════════════════════════════════════════ */
void menu_display_process(void) {
    /* 检测任务是否已自行完成 */
    if (menu_state == MENU_RUNNING && STATE_MACHINE.Main_State == STOP_STATE) {
        menu_state    = MENU_STOPPED;
        stepper_goto_horizontal();      /* 任务完成, 摆杆归零 */
        display_dirty = 1;
    }

    /* 运行中不再定时刷新, 避免 OLED SPI busy-wait 干扰编码器采样 */
    /* 显示仅在按键/状态切换时更新, 任务完成后自动显示最终结果 */

    if (display_dirty) {
        menu_display();
    }
}

void menu_display(void) {
    switch (menu_state) {
    case MENU_SELECTING: draw_selecting(); break;
    case MENU_RUNNING:   draw_running();   break;
    case MENU_STOPPED:   draw_stopped();   break;
    }
    display_dirty = 0;
}

/* ═══════════════════════════════════════════════════════════════════
 * 工具函数
 * ═══════════════════════════════════════════════════════════════════ */

static void format_time(uint32_t ms, char *buf, uint8_t size) {
    uint16_t total_s  = ms / 1000;
    uint8_t  tenth    = (ms % 1000) / 100;
    uint8_t  min      = total_s / 60;
    uint8_t  sec      = total_s % 60;

    if (min > 0) {
        snprintf(buf, size, "%d:%02d.%d", min, sec, tenth);
    } else {
        snprintf(buf, size, "%d.%ds", sec, tenth);
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * 绘制函数  — 字体大小16 (8x16), 4行显示
 * ═══════════════════════════════════════════════════════════════════ */

/**
 * @brief 选题画面：5项滚动, 4行可见, 选中项前加 '>' 光标
 */
static void draw_selecting(void) {
    char line[18];

    /* 先清 GRAM (不刷新), 再绘制, 最后一次性刷新 */
    OLED_ClearGram();

    /* 滚动偏移: 5项中4行可见, 选中第5项(索引4)时窗口下移1行 */
    uint8_t scroll = (menu_selected_index >= 4) ? 1 : 0;

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t task_idx = scroll + i;
        if (task_idx >= 5) break;

        line[0] = (task_idx == menu_selected_index) ? '>' : ' ';
        snprintf(line + 1, sizeof(line) - 1, "%s", task_labels[task_idx]);
        OLED_ShowString(0, i * 16, (uint8_t *)line, 16, 1);
    }

    OLED_Refresh();
}

/**
 * @brief 运行画面：任务名 + 计时
 */
static void draw_running(void) {
    char line[18];

    OLED_ClearGram();

    snprintf(line, sizeof(line), "%s", task_labels[menu_selected_index]);
    OLED_ShowString(0, 0, (uint8_t *)line, 16, 1);

    if (menu_selected_index == 1) {
        snprintf(line, sizeof(line), "Running...");
        OLED_ShowString(0, 16, (uint8_t *)line, 16, 1);
    } else {
        if (lap_start_tick > 0) {
            uint32_t elapsed = sys_tick_ms - lap_start_tick;
            format_time(elapsed, line, sizeof(line));
            OLED_ShowString(0, 16, (uint8_t *)line, 16, 1);
        }
    }

    OLED_Refresh();
}

/**
 * @brief 停止画面：完成提示 + 最终计时
 */
static void draw_stopped(void) {
    char line[18];

    OLED_ClearGram();

    if (menu_selected_index == 1) {
        snprintf(line, sizeof(line), "%s", task_labels[1]);
        OLED_ShowString(0, 0, (uint8_t *)line, 16, 1);
        snprintf(line, sizeof(line), "Done");
        OLED_ShowString(0, 16, (uint8_t *)line, 16, 1);
    } else {
        snprintf(line, sizeof(line), "%s", task_labels[menu_selected_index]);
        OLED_ShowString(0, 0, (uint8_t *)line, 16, 1);

        if (lap_stop_tick > lap_start_tick) {
            uint32_t total = lap_stop_tick - lap_start_tick;
            format_time(total, line, sizeof(line));
            OLED_ShowString(0, 16, (uint8_t *)line, 16, 1);
        }
    }

    OLED_Refresh();
}
