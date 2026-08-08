#ifndef __POSITION_H
#define __POSITION_H
#include "headfile.h"

#define PI                      3.1415926535f    // 圆周率

// 轮子直径 4.8cm，半径 2.4cm
#define WHEEL_RADIUS_CM          2.4f            // 轮子半径（厘米）
#define POSITION_SAMPLING_INTERVAL_MS  10      // 位置环采样间隔（毫秒）

// 位置环PD参数
#define POSITION_KP              20.0f          // 位置环比例系数
#define POSITION_KD              1.0f           // 位置环微分系数
#define POSITION_OUTPUT_MAX      150.0f         // 位置环最大输出速度（RPM）
#define POSITION_OUTPUT_MIN      -150.0f        // 位置环最小输出速度（RPM）

extern PID position_pid_left;   // 左轮位置PID控制器
extern PID position_pid_right;  // 右轮位置PID控制器
extern double current_position_left;    // 左轮累计位置（厘米）
extern double current_position_right;   // 右轮累计位置（厘米）
extern double target_position;          // 目标位置（厘米）

/**
 * @brief 获取轮子周长（厘米）
 * @return 轮子周长
 */
float position_get_wheel_circumference(void);

/**
 * @brief 位置环PD计算函数
 * @param pid PID控制器指针
 * @param current_pos 当前位置（厘米）
 * @param target_pos 目标位置（厘米）
 * @return PD输出值（目标速度，RPM）
 */
float position_pd_calc(PID *pid, double current_pos, double target_pos);

/**
 * @brief 更新位置（根据编码器计数值计算并累加位置）
 */
void position_update(void);

/**
 * @brief 位置控制函数
 * @param target_pos 目标位置（厘米）
 */
void position_control(double target_pos);

/**
 * @brief 重置位置为零
 */
void position_reset(void);

#endif