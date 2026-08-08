/**
 * @file question_task.c
 * @brief 竞赛题目任务实现文件
 * @details 包含4个竞赛题目的状态机任务实现，分别对应不同的机器人运动场景
 * @author [Your Name]
 * @date [Date]
 */

#include "question_task.h"
#include "headfile.h"


// 全局状态机变量，管理主状态和各题目状态
state_machine STATE_MACHINE;

// 各题目首次执行标志，用于控制初始化操作
uint8_t q1_first_flag = 0;  // 题目1首次执行标志
uint8_t q2_first_flag = 0;  // 题目2首次执行标志
uint8_t q3_first_flag = 0;  // 题目3首次执行标志
uint8_t q4_first_flag = 0;  // 题目4首次执行标志

// ========== 题目1状态定义 ==========
#define Q1_State_1 1  // 初始状态，开始转弯
#define Q1_State_2 2  // 等待检测赛道

// ========== 题目2状态定义 ==========
#define Q2_State_1 1  // 初始状态，等待检测赛道
#define Q2_State_2 2  // 循迹前进
#define Q2_State_3 3  // 原地转向180度
#define Q2_State_4 4  // 循迹返回
#define Q2_State_5 5  // 完成任务，停止

// ========== 题目3状态定义 ==========
#define Q3_State_1 1  // 初始状态，直行一段距离
#define Q3_State_2 2  // 等待检测赛道
#define Q3_State_3 3  // 循迹前进
#define Q3_State_4 4  // 直行一段距离
#define Q3_State_5 5  // 原地转向180度
#define Q3_State_6 6  // 循迹返回
#define Q3_State_7 7  // 完成任务，停止

// ========== 题目4状态定义 ==========
#define Q4_State_1 1  // 初始状态，直行一段距离
#define Q4_State_2 2  // 等待检测赛道
#define Q4_State_3 3  // 循迹前进
#define Q4_State_4 4  // 直行一段距离
#define Q4_State_5 5  // 原地转向180度
#define Q4_State_6 6  // 循迹返回
#define Q4_State_7 7  // 圈数计数与判断

// 题目4圈数计数器（完成4圈后停止）
uint8_t q4_lap_count = 0;

/**
 * @brief 题目任务初始化函数
 * @details 初始化状态机的所有状态为初始状态
 */
void question_task_init(void) {
    STATE_MACHINE.Main_State = STOP_STATE;  // 主状态设为停止状态
    STATE_MACHINE.Q1_State = Q1_State_1;    // 题目1状态初始化
    STATE_MACHINE.Q2_State = Q2_State_1;    // 题目2状态初始化
    STATE_MACHINE.Q3_State = Q3_State_1;    // 题目3状态初始化
    STATE_MACHINE.Q4_State = Q4_State_1;    // 题目4状态初始化
}

/**
 * @brief 题目1任务函数
 * @details 实现原地旋转等待检测赛道的功能
 *          状态机流程：Q1_State_1(开始转弯) -> Q1_State_2(等待检测赛道) -> 检测到赛道后停止
 */
void question1_task(void) {
    if(STATE_MACHINE.Q1_State == Q1_State_1){
        // 状态1：开始原地旋转，速度150，角度0(持续旋转)
        turn_control(target_speed, 0.0f);
        STATE_MACHINE.Q1_State = Q1_State_2;  // 切换到状态2
        buzzer_time_ms = 500;                  // 蜂鸣器提示状态切换
    }else if(STATE_MACHINE.Q1_State == Q1_State_2){
        // 状态2：持续旋转并检测赛道
        turn_control(target_speed, 0.0f);
        if(track_read() != 0){  // 检测到赛道
            buzzer_time_ms = 500;              // 蜂鸣器提示
            speed_control(0.0f, 0.0f);         // 停止电机
            STATE_MACHINE.Main_State = STOP_STATE;  // 主状态设为停止
            STATE_MACHINE.Q1_State = Q1_State_1;    // 重置题目1状态
        }
    }
}

/**
 * @brief 题目2任务函数
 * @details 实现原地旋转找赛道 -> 循迹前进 -> 180度转向 -> 循迹返回的功能
 *          状态机流程：
 *          Q2_State_1(原地旋转找赛道) -> Q2_State_2(循迹前进)
 *          -> Q2_State_3(180度转向) -> Q2_State_4(循迹返回)
 *          -> Q2_State_5(完成任务)
 */
void question2_task(void) {
    if(STATE_MACHINE.Q2_State == Q2_State_1){
        // 状态1：原地旋转寻找赛道
        if(q2_first_flag == 0){
            q2_first_flag = 1;      // 标记首次执行
            buzzer_time_ms = 500;   // 蜂鸣器提示开始
        }
        turn_control(target_speed, 0.0f);  // 持续旋转
        if(track_read() != 0){       // 检测到赛道
            STATE_MACHINE.Q2_State = Q2_State_2;  // 切换到循迹状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q2_State == Q2_State_2) {
        // 状态2：循迹前进
        track_control(target_speed);       // 循迹控制，速度150
        if(track_read() == 0){       // 检测不到赛道（到达终点）
            STATE_MACHINE.Q2_State = Q2_State_3;  // 切换到转向状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if(STATE_MACHINE.Q2_State == Q2_State_3){
        // 状态3：原地180度转向
        turn_control(target_speed, -179.99f);  // 转向-179.99度（接近180度）
        if(track_read() != 0){            // 检测到赛道
            STATE_MACHINE.Q2_State = Q2_State_4;  // 切换到返回循迹状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if(STATE_MACHINE.Q2_State == Q2_State_4){
        // 状态4：循迹返回
        track_control(target_speed);       // 循迹控制返回
        if(track_read() == 0){       // 检测不到赛道（回到起点）
            STATE_MACHINE.Q2_State = Q2_State_5;  // 切换到完成状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q2_State == Q2_State_5) {
        // 状态5：完成任务，停止
        speed_control(0.0f, 0.0f);              // 停止电机
        STATE_MACHINE.Main_State = STOP_STATE;  // 主状态设为停止
        STATE_MACHINE.Q2_State = Q2_State_1;    // 重置题目2状态
        buzzer_time_ms = 500;                   // 蜂鸣器提示完成
    }
}

/**
 * @brief 题目3任务函数
 * @details 实现直行 -> 原地旋转找赛道 -> 循迹前进 -> 直行 -> 180度转向 -> 循迹返回的功能
 *          状态机流程：
 *          Q3_State_1(直行84cm) -> Q3_State_2(原地旋转找赛道)
 *          -> Q3_State_3(循迹前进) -> Q3_State_4(直行96cm)
 *          -> Q3_State_5(180度转向) -> Q3_State_6(循迹返回)
 *          -> Q3_State_7(完成任务)
 */
void question3_task(void) {
    if(STATE_MACHINE.Q3_State == Q3_State_1){
        // 状态1：直行84cm
        if(q3_first_flag == 0){
            q3_first_flag = 1;      // 标记首次执行
            buzzer_time_ms = 500;   // 蜂鸣器提示开始
        }
        // 以速度150、转向角60度直行，获取已行驶距离
        float distance = turn_control_get_distance(target_speed, 60.0f);
        if(distance >= 96.0f){      // 达到目标距离84cm
            turn_control_reset_distance();         // 重置距离计数器
            STATE_MACHINE.Q3_State = Q3_State_2;  // 切换到状态2
        }
    }else if (STATE_MACHINE.Q3_State == Q3_State_2) {
        // 状态2：原地旋转寻找赛道
        turn_control(target_speed, 0.0f);  // 持续旋转
        if(track_read() != 0){       // 检测到赛道
            STATE_MACHINE.Q3_State = Q3_State_3;  // 切换到循迹状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if(STATE_MACHINE.Q3_State == Q3_State_3){
        // 状态3：循迹前进
        track_control(target_speed);       // 循迹控制，速度150
        if(track_read() == 0){       // 检测不到赛道
            STATE_MACHINE.Q3_State = Q3_State_4;  // 切换到直行状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q3_State == Q3_State_4) {
        // 状态4：直行96cm
        // 以速度150、转向角120度直行，获取已行驶距离
        float distance = turn_control_get_distance(target_speed, 120.0f);
        if(distance >= 96.0f){      // 达到目标距离96cm
            turn_control_reset_distance();         // 重置距离计数器
            STATE_MACHINE.Q3_State = Q3_State_5;  // 切换到转向状态
        }
    }else if (STATE_MACHINE.Q3_State == Q3_State_5) {
        // 状态5：原地180度转向
        turn_control(target_speed, -179.99f);  // 转向-179.99度
        if(track_read() != 0){            // 检测到赛道
            STATE_MACHINE.Q3_State = Q3_State_6;  // 切换到返回循迹状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q3_State == Q3_State_6) {
        // 状态6：循迹返回
        track_control(target_speed);       // 循迹控制返回
        if(track_read() == 0){       // 检测不到赛道（回到起点）
            STATE_MACHINE.Q3_State = Q3_State_7;  // 切换到完成状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q3_State == Q3_State_7) {
        // 状态7：完成任务，停止
        speed_control(0.0f, 0.0f);              // 停止电机
        STATE_MACHINE.Main_State = STOP_STATE;  // 主状态设为停止
        STATE_MACHINE.Q3_State = Q3_State_1;    // 重置题目3状态
    }
}

/**
 * @brief 题目4任务函数
 * @details 实现题目3的循环版本，完成4圈后停止
 *          状态机流程与题目3相同，但在Q4_State_7进行圈数判断
 *          完成4圈后停止，否则继续下一圈
 */
void question4_task(void) {
    if(STATE_MACHINE.Q4_State == Q4_State_1){
        // 状态1：直行84cm
        if(q4_first_flag == 0){
            q4_first_flag = 1;      // 标记首次执行
            buzzer_time_ms = 500;   // 蜂鸣器提示开始
        }
        // 以速度150、转向角60度直行，获取已行驶距离
        float distance = turn_control_get_distance(target_speed, 60.0f);
        if(distance >= 96.0f){      // 达到目标距离84cm
            turn_control_reset_distance();         // 重置距离计数器
            STATE_MACHINE.Q4_State = Q4_State_2;  // 切换到状态2
        }
    }else if (STATE_MACHINE.Q4_State == Q4_State_2) {
        // 状态2：原地旋转寻找赛道
        turn_control(target_speed, 0.0f);  // 持续旋转
        if(track_read() != 0){       // 检测到赛道
            STATE_MACHINE.Q4_State = Q4_State_3;  // 切换到循迹状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if(STATE_MACHINE.Q4_State == Q4_State_3){
        // 状态3：循迹前进
        track_control(target_speed);       // 循迹控制，速度150
        if(track_read() == 0){       // 检测不到赛道
            STATE_MACHINE.Q4_State = Q4_State_4;  // 切换到直行状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q4_State == Q4_State_4) {
        // 状态4：直行96cm
        // 以速度150、转向角120度直行，获取已行驶距离
        float distance = turn_control_get_distance(target_speed, 120.0f);
        if(distance >= 96.0f){      // 达到目标距离96cm
            turn_control_reset_distance();         // 重置距离计数器
            STATE_MACHINE.Q4_State = Q4_State_5;  // 切换到转向状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q4_State == Q4_State_5) {
        // 状态5：原地180度转向
        turn_control(target_speed, -179.99f);  // 转向-179.99度
        if(track_read() != 0){            // 检测到赛道
            STATE_MACHINE.Q4_State = Q4_State_6;  // 切换到返回循迹状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q4_State == Q4_State_6) {
        // 状态6：循迹返回
        track_control(target_speed);       // 循迹控制返回
        if(track_read() == 0){       // 检测不到赛道（回到起点）
            STATE_MACHINE.Q4_State = Q4_State_7;  // 切换到圈数判断状态
            buzzer_time_ms = 500;                  // 蜂鸣器提示
        }
    }else if (STATE_MACHINE.Q4_State == Q4_State_7) {
        // 状态7：圈数计数与判断
        q4_lap_count++;              // 圈数加1
        buzzer_time_ms = 500;        // 蜂鸣器提示
        
        if(q4_lap_count >= 4){       // 判断是否完成4圈
            speed_control(0.0f, 0.0f);              // 停止电机
            STATE_MACHINE.Main_State = STOP_STATE;  // 主状态设为停止
            STATE_MACHINE.Q4_State = Q4_State_1;    // 重置题目4状态
            q4_lap_count = 0;                       // 重置圈数计数器
        }else{
            // 未完成4圈，继续下一圈
            STATE_MACHINE.Q4_State = Q4_State_1;    // 回到状态1重新开始
        }
    }
}