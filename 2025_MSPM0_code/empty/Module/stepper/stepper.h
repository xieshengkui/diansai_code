#ifndef __STEPPER_H_
#define __STEPPER_H_

#include "headfile.h"

typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float err;
    float err_sum;
    float err_diff;
    float last_err;
} PID;

// 屏幕尺寸宏
#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  480

// 屏幕中心点宏（左上角坐标系）
#define SCREEN_CENTER_X  (SCREEN_WIDTH  / 2)
#define SCREEN_CENTER_Y  (SCREEN_HEIGHT / 2)

/* 位置式PID参数 */
#define STEPPER1_KP         0.5f
#define STEPPER1_KI         0.0f
#define STEPPER1_KD         0.3f

#define STEPPER2_KP         0.08f
#define STEPPER2_KI         0.0f
#define STEPPER2_KD         0.032f

/* 数据看门狗 */
#define K230_TIMEOUT_MS    200     // K230断连超时(ms)

/* 保护阈值 */
#define STEPPER_OUT_MAX    30.0f    // 输出限幅 ±30 RPM
#define STEPPER_INT_LIMIT  1.5f     // 积分限幅 ±1.5
#define STEPPER_DEADBAND   6.0f     // 输入死区: |error| < 6px 视为0


extern int x_error;
extern int y_error;
extern void stepper_write(uint8_t *data, uint16_t size);
void Emm_V5_Reset_Motor(uint8_t addr);
void Emm_V5_Reset_CurPos_To_Zero(uint8_t addr);
void Emm_V5_En_Control(uint8_t addr, bool state, bool snF);
void Emm_V5_Vel_Control_1(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);
void Emm_V5_Vel_Control_2(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);
void Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, uint8_t raF, bool snF);
void Emm_V5_Stop_Now(uint8_t addr, bool snF);

/* 位置式PID计算 */
float stepper1_pd_calc(PID *pid, int x_error);
float stepper2_pd_calc(PID *pid, int x_error);
void follow_control(void);

#endif /* INC_COMMAND_H_ */
