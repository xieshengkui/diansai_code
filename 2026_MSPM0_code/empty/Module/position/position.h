#ifndef __POSITION_H
#define __POSITION_H
#include "headfile.h"

#define WHEEL_RADIUS_CM          3.46f            // 轮子半径（厘米）
#define WHEEL_CIRCUMFERENCE_CM   (2.0f * PI * WHEEL_RADIUS_CM)  // 轮子周长（厘米）

// 位置环PD参数
#define POSITION_KP              20.0f          // 位置环比例系数
#define POSITION_KD              1.0f           // 位置环微分系数
#define POSITION_OUTPUT_MAX      50.0f         // 位置环最大输出速度（RPM）
#define POSITION_OUTPUT_MIN      -50.0f        // 位置环最小输出速度（RPM）

extern PID position_pid_left;   // 左轮位置PID控制器
extern PID position_pid_right;  // 右轮位置PID控制器
extern float current_position_left;    // 左轮累计位置（厘米）
extern float current_position_right;   // 右轮累计位置（厘米）
extern float target_position;          // 目标位置（厘米）

/**
 * @brief 位置环PD计算函数
 * @param pid PID控制器指针
 * @param current_pos 当前位置（厘米）
 * @param target_pos 目标位置（厘米）
 * @return PD输出值（目标速度，RPM）
 */
float position_pd_calc(PID *pid, float current_pos, float target_pos);

/**
 * @brief 更新位置（根据编码器计数值计算并累加位置）
 */
void position_update(void);

/**
 * @brief 位置控制函数
 * @param target_pos 目标位置（厘米）
 */
void position_control(float target_pos);

/**
 * @brief 重置位置为零
 */
void position_reset(void);

#endif