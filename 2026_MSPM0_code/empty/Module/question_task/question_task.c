/**
 * @file question_task.c
 * @brief 竞赛题目任务实现
 * @details Q1=整圈启停线(≤20s), Q2=钢球静态控制 O→+5→-5(≤5s),
 *          Q3=A→B+球平衡(≤8s), Q4=整圈+球在中心(≤30s),
 *          Q5=整圈+球在任意位置(≤30s)
 * @note   所有移动任务(Q3/Q4/Q5)均调用 ball_balance_control() 控制钢球
 */

#include "question_task.h"
#include "headfile.h"

extern int32_t  k230_pos;
extern int8_t flag1;
extern int8_t flag2;
extern uint8_t acc_flag;

float dist = 0.0f;

float q5_target = 0.0f;       /* Q5 锁定的球目标位置, task[1] 调用 */

/* ── 全局状态机 ── */
state_machine STATE_MACHINE;

/* ── 首次执行标志 ── */
uint8_t q2_first_flag = 0;
uint8_t q3_first_flag = 0;
uint8_t q4_first_flag = 0;
uint8_t q5_first_flag = 0;
uint8_t q4_lap_count  = 0;

/* ═══════════════════════════════════════════════════════════════════
 * 初始化
 * ═══════════════════════════════════════════════════════════════════ */
void question_task_init(void) {
    STATE_MACHINE.Main_State = STOP_STATE;
    STATE_MACHINE.Q1_State  = Q1_State_1;
    STATE_MACHINE.Q2_State  = Q2_State_1;
    STATE_MACHINE.Q3_State  = Q3_State_1;
    STATE_MACHINE.Q4_State  = Q4_State_1;
    STATE_MACHINE.Q5_State  = Q5_State_1;
}

/* ═══════════════════════════════════════════════════════════════════
 * Task1: 整圈循线 ≤20s (要求2)
 * 小车放启停线前 → 第1次检测(≥4/8黑线)→计时开始+距离清零
 * → 第2次检测且dist>600 → 停车, 或 dist>630 兜底停车
 * ═══════════════════════════════════════════════════════════════════ */
void question1_task(void) {
    static uint8_t was_on_line = 0;
    static uint8_t hit_count   = 0;

    if (STATE_MACHINE.Q1_State == Q1_State_1) {
        was_on_line = 0;
        hit_count   = 0;
        lap_start_tick = 0;
        lap_stop_tick  = 0;
        track_control_reset_distance();
        STATE_MACHINE.Q1_State = Q1_State_2;

    } else if (STATE_MACHINE.Q1_State == Q1_State_2) {
        dist = track_control_q1_get_distance(TASK1_SPEED);

        uint8_t on_line = track_detect_start_line_cached();

        /* ── 上升沿检测启停线 ── */
        if (on_line && !was_on_line) {
            hit_count++;
            if (hit_count == 1) {
                lap_start_tick = sys_tick_ms;
                track_control_reset_distance();  /* 从启停线开始计距离 */
                dist = 0.0f;
            }
        }
        was_on_line = on_line;

        /* ── 停车条件: 第2次检测且dist>600, 或兜底dist>630 ── */
        if (hit_count >= 1) {
            if ((hit_count >= 2 && dist > 580.0f) || (dist > 1000.0f)) {
                lap_stop_tick = sys_tick_ms;
                speed_control(0.0f, 0.0f);
                STATE_MACHINE.Q1_State = Q1_State_3;
            }
        }

    } else if (STATE_MACHINE.Q1_State == Q1_State_3) {
        STATE_MACHINE.Main_State = STOP_STATE;
        STATE_MACHINE.Q1_State  = Q1_State_1;
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Task2: 钢球静态位置控制 O→+50mm→-50mm ≤5s (要求3)
 * 小车不动, flag1: 0=去+50mm, 1=去-50mm → 完成回STOP
 * ═══════════════════════════════════════════════════════════════════ */
void question2_task(void) {
    acc_flag = 0;
    if (flag1 == 0) {
        pos_error = 50 - k230_pos;
        ball_balance_control();
        if (k230_pos <= 60 && k230_pos >= 40) {
            flag1 = 1;
        }
    } else {
        pos_error = -50 - k230_pos;
        ball_balance_control();
        /* 一直稳定在-50mm, KEY4 退出 */
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Task3: A→B 循线 + 球平衡 ≤8s (要求4)
 * 加速→巡航→过B(150cm)记录时间→一直匀速, KEY4退出
 * ═══════════════════════════════════════════════════════════════════ */
void question3_task(void) {
    // ── 旧版 (有减速+停车) ──
    // if (STATE_MACHINE.Q3_State == Q3_State_1) {
    //     track_control_reset_distance();
    //     pos_error = - k230_pos;
    //     ball_balance_control();
    //     lap_start_tick = sys_tick_ms;
    //     STATE_MACHINE.Q3_State = Q3_State_2;
    // } else if (STATE_MACHINE.Q3_State == Q3_State_2) {
    //     float dist = track_control_smooth(200.0f, 80.0f, 0.15f);
    //     pos_error = - k230_pos;
    //     ball_balance_control();
    //     if (dist >= 150.0f && lap_stop_tick == 0) {
    //         lap_stop_tick = sys_tick_ms;
    //     }
    //     if (dist >= 200.0f) {
    //         speed_control(0.0f, 0.0f);
    //         STATE_MACHINE.Q3_State = Q3_State_3;
    //     }
    // } else if (STATE_MACHINE.Q3_State == Q3_State_3) {
    //     pos_error = - k230_pos;
    //     ball_balance_control();
    // }

    // ── 新版: 不对称梯形, 加/减速独立, B后5cm开始减速 ──
    acc_flag = 1;
    if (STATE_MACHINE.Q3_State == Q3_State_1) {
        track_control_reset_distance();
        lap_start_tick = sys_tick_ms;
        STATE_MACHINE.Q3_State = Q3_State_2;

    } else if (STATE_MACHINE.Q3_State == Q3_State_2) {
        float dist = track_control_asymmetric(250.0f, 105.0f, 60.0f, 90.0f);

        if (dist >= 150.0f && lap_stop_tick == 0) {
            lap_stop_tick = sys_tick_ms;
        }
        if (dist >= 250.0f) {
            speed_control(0.0f, 0.0f);
            STATE_MACHINE.Q3_State = Q3_State_3;
        }

    } else if (STATE_MACHINE.Q3_State == Q3_State_3) {
        /* 已停车, 等 KEY4 */
    }
}

    
//         pos_error = - k230_pos;
//         ball_balance_control();
//         if (STATE_MACHINE.Q3_State == Q3_State_1) {
//         track_control_reset_distance();
//         lap_start_tick = sys_tick_ms;
//         STATE_MACHINE.Q3_State = Q3_State_2;

//     } else if (STATE_MACHINE.Q3_State == Q3_State_2) {
//         float dist = track_control_get_distance(TASK3_SPEED);
//         if (dist >= TASK3_TARGET_DISTANCE) {
//             lap_stop_tick = sys_tick_ms;
//             speed_control(0.0f, 0.0f);
//             STATE_MACHINE.Q3_State = Q3_State_3;
//         }

//     } else if (STATE_MACHINE.Q3_State == Q3_State_3) {
//         STATE_MACHINE.Main_State = STOP_STATE;
//         STATE_MACHINE.Q3_State  = Q3_State_1;
//     }
// }

/* ═══════════════════════════════════════════════════════════════════
 * Task4: 整圈循线 + 球平衡 ≤30s (要求5)
 * 不对称梯形: 加速160cm → 巡航 → 过线(614cm) → 继续10cm+ → 减速100cm → 停车(750cm)
 * 加速/减速独立控制, 线在巡航区, 停车在弯道前(764cm)
 * ═══════════════════════════════════════════════════════════════════ */
void question4_task(void) {
    acc_flag = 1;
    if (STATE_MACHINE.Q4_State == Q4_State_1) {
        track_control_reset_distance();
        lap_start_tick = sys_tick_ms;
        lap_stop_tick  = 0;
        STATE_MACHINE.Q4_State = Q4_State_2;

    } else if (STATE_MACHINE.Q4_State == Q4_State_2) {
        float dist = track_control_asymmetric(900.0f, 85.0f, 160.0f, 150.0f);

        if (track_detect_middle4_cached() && dist > 500.0f && lap_stop_tick == 0) {
            lap_stop_tick = sys_tick_ms;
        }
        if (dist >= 750.0f) {
            speed_control(0.0f, 0.0f);
            STATE_MACHINE.Q4_State = Q4_State_3;
        }

    } else if (STATE_MACHINE.Q4_State == Q4_State_3) {
        /* 已停车, 等 KEY4 */
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Task5: 整圈循线 + 球在任意位置 ≤30s (要求6)
 * 两阶段: 第1次KEY3→停球+水管归零→评委放球→第2次KEY3→采5次均值→跑一圈
 * ═══════════════════════════════════════════════════════════════════ */
void question5_task(void) {
    acc_flag = 1;
    static int   sample_cnt = 0;
    static float sample_sum = 0.0f;

    if (STATE_MACHINE.Q5_State == Q5_State_1) {
        speed_control(0.0f, 0.0f);
        lap_start_tick = 0;
        lap_stop_tick  = 0;

    } else if (STATE_MACHINE.Q5_State == Q5_State_2) {
        speed_control(0.0f, 0.0f);
        sample_sum += k230_pos;
        sample_cnt++;
        if (sample_cnt >= 5) {
            q5_target  = sample_sum / 5.0f;
            sample_cnt = 0;
            sample_sum = 0.0f;
            track_control_reset_distance();
            lap_start_tick = sys_tick_ms;
            ball_pid_set_q5();
            STATE_MACHINE.Q5_State = Q5_State_3;
        }

    } else if (STATE_MACHINE.Q5_State == Q5_State_3) {
        float dist = track_control_asymmetric(900.0f, 85.0f, 160.0f, 150.0f);

        if (track_detect_start_line_cached() && dist > 500.0f && lap_stop_tick == 0) {
            lap_stop_tick = sys_tick_ms;
        }
        if (dist >= 750.0f) {
            speed_control(0.0f, 0.0f);
            STATE_MACHINE.Q5_State = Q5_State_4;
        }

    } else if (STATE_MACHINE.Q5_State == Q5_State_4) {
        STATE_MACHINE.Main_State = STOP_STATE;
        STATE_MACHINE.Q5_State  = Q5_State_1;
    }
}
