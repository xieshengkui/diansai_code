#ifndef QUESTION_TASK_H
#define QUESTION_TASK_H

#include "headfile.h"

/* ── 各任务目标速度 (RPM) ── */
#define TASK1_SPEED  100.0f   // 整圈 ≤20s
#define TASK3_SPEED   80.0f   // A→B 1.5m ≤8s
#define TASK4_SPEED   65.0f   // 整圈 ≤30s
#define TASK5_SPEED   65.0f   // 整圈 ≤30s

/* ── Task3 A→B 目标距离 (cm) ── */
#define TASK3_TARGET_DISTANCE  170.0f

/* ── 函数声明 ── */
void question1_task(void);
void question2_task(void);
void question3_task(void);
void question4_task(void);
void question5_task(void);

/* ── 状态机结构体 ── */
typedef struct {
    int Main_State;
    int Q1_State;
    int Q2_State;
    int Q3_State;
    int Q4_State;
    int Q5_State;
} state_machine;

/* ── 主状态 ── */
#define STOP_STATE  0
#define Q1_STATE    1
#define Q2_STATE    2
#define Q3_STATE    3
#define Q4_STATE    4
#define Q5_STATE    5

/* Task1: 整圈启停线检测 (4状态) */
#define Q1_State_1  1
#define Q1_State_2  2
#define Q1_State_3  3
#define Q1_State_4  4

/* Task2: 钢球静态位置控制 O→+50mm→-50mm (4状态) */
#define Q2_State_1  1   /* 初始化: 设目标=+50mm */
#define Q2_State_2  2   /* 等待到达+50mm */
#define Q2_State_3  3   /* 折返: 设目标=-50mm, 等待到达并稳定 */
#define Q2_State_4  4   /* 完成, 返回STOP */

/* Task3: A→B 距离停止 + 球平衡 (3状态) */
#define Q3_State_1  1
#define Q3_State_2  2
#define Q3_State_3  3

/* Task4: 整圈循线 + 球在中心 (4状态) */
#define Q4_State_1  1
#define Q4_State_2  2
#define Q4_State_3  3
#define Q4_State_4  4

/* Task5: 整圈循线 + 球在任意位置 */
#define Q5_State_1  1   /* 等待: 停球, 水管归零, 等评委放球后按KEY3 */
#define Q5_State_2  2   /* 采样: 采5次k230_pos求平均作为目标 */
#define Q5_State_3  3   /* 循线+球平衡: 跑一圈 */
#define Q5_State_4  4   /* 完成 */

/* ── 外部变量 ── */
extern uint8_t q2_first_flag;
extern uint8_t q3_first_flag;
extern uint8_t q4_first_flag;
extern uint8_t q5_first_flag;
extern uint8_t q4_lap_count;
extern state_machine STATE_MACHINE;
extern float q5_target;       /* Q5 锁定的球目标位置 */

void question_task_init(void);

#endif