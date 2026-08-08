/**
 * @file stepper.c
 * @brief 串级PID 球杆平衡 — 外环位置 + 内环球速
 * @details 外环: pos_error(mm) → PID → 目标球速(mm/s)
 *          内环: (目标球速 - 实际球速) → PID → 水管角度(°) → 脉冲 → 电机
 *          核心: 球速越快, 内环提前抵消倾斜 → 到达目标时速度≈0 → 不超调
 */

#include "headfile.h"

float target_vel_debug;
uint8_t acc_flag = 0;
float kf = 0;
extern volatile float acc;
/* ── 外环 PID ── */
static PID pos_pid  = {POS_KP, POS_KI, POS_KD, 0.0f, 0.0f, 0.0f, 0.0f};

/* ── 内环 PID ── */
static PID vel_pid  = {VEL_KP, VEL_KI, VEL_KD, 0.0f, 0.0f, 0.0f, 0.0f};

/* ── 全局变量 ── */
float ball_velocity           = 0.0f;   /* K230 'V' 回传 */
float ball_target_angle_debug = 0.0f;   /* 调参: 目标角度 */

/* ═══════════════════════════════════════════════════════════════════
 * 底层通信
 * ═══════════════════════════════════════════════════════════════════ */

void stepper_write(uint8_t *data, uint16_t size)
{
    DL_DMA_setSrcAddr(DMA, stepper_DMA_CHAN_ID, (uint32_t)data);
    DL_DMA_setDestAddr(DMA, stepper_DMA_CHAN_ID,
        (uint32_t)(&stepper_INST->TXDATA));
    DL_DMA_setTransferSize(DMA, stepper_DMA_CHAN_ID, size);
    DL_DMA_enableChannel(DMA, stepper_DMA_CHAN_ID);
}

/* ═══════════════════════════════════════════════════════════════════
 * 电机指令
 * ═══════════════════════════════════════════════════════════════════ */

void Emm_V5_Reset_Motor(uint8_t addr)
{
    static uint8_t cmd[16] = {0};
    cmd[0] = addr;
    cmd[1] = 0x08; cmd[2] = 0x97; cmd[3] = 0x6B;
    stepper_write((uint8_t*)cmd, 4);
}

void Emm_V5_Reset_CurPos_To_Zero(uint8_t addr)
{
    static uint8_t cmd[16] = {0};
    cmd[0] = addr;
    cmd[1] = 0x0A; cmd[2] = 0x6D; cmd[3] = 0x6B;
    stepper_write((uint8_t*)cmd, 4);
}

void Emm_V5_En_Control(uint8_t addr, bool state, bool snF)
{
    static uint8_t cmd[16] = {0};
    cmd[0] = addr; cmd[1] = 0xF3; cmd[2] = 0xAB;
    cmd[3] = (uint8_t)state;
    cmd[4] = snF;  cmd[5] = 0x6B;
    stepper_write((uint8_t*)cmd, 6);
}

void Emm_V5_Vel_Control_1(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
    static uint8_t cmd[16] = {0};
    cmd[0] = addr; cmd[1] = 0xF6; cmd[2] = dir;
    cmd[3] = (uint8_t)(vel >> 8);
    cmd[4] = (uint8_t)(vel >> 0);
    cmd[5] = acc;  cmd[6] = snF;  cmd[7] = 0x6B;
    stepper_write((uint8_t*)cmd, 8);
}

void Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc,
                        uint32_t clk, uint8_t raF, bool snF)
{
    __IO static uint8_t cmd[16] = {0};
    cmd[0]  = addr;
    cmd[1]  = 0xFD;
    cmd[2]  = dir;
    cmd[3]  = (uint8_t)(vel >> 8);
    cmd[4]  = (uint8_t)(vel >> 0);
    cmd[5]  = acc;
    cmd[6]  = (uint8_t)(clk >> 24);
    cmd[7]  = (uint8_t)(clk >> 16);
    cmd[8]  = (uint8_t)(clk >> 8);
    cmd[9]  = (uint8_t)(clk >> 0);
    cmd[10] = raF;
    cmd[11] = snF;
    cmd[12] = 0x6B;
    stepper_write((uint8_t*)cmd, 13);
}

void Emm_V5_Stop_Now(uint8_t addr, bool snF)
{
    static uint8_t cmd[16] = {0};
    cmd[0] = addr; cmd[1] = 0xFE; cmd[2] = 0x98;
    cmd[3] = snF;  cmd[4] = 0x6B;
    stepper_write((uint8_t*)cmd, 5);
}

/* ═══════════════════════════════════════════════════════════════════
 * 初始化
 * ═══════════════════════════════════════════════════════════════════ */

void stepper_init(void)
{
    DL_DMA_enableChannel(DMA, stepper_DMA_CHAN_ID);
    delay_ms(50);
    Emm_V5_En_Control(STEPPER_ADDR, true, false);
    delay_ms(10);
    Emm_V5_Reset_CurPos_To_Zero(STEPPER_ADDR);
}

void stepper_goto_horizontal(void)
{
    Emm_V5_Pos_Control(STEPPER_ADDR, 1, STEPPER_POS_VEL, STEPPER_POS_ACC, 0, 1, false);
}

/* ═══════════════════════════════════════════════════════════════════
 * PID参数集切换
 * ═══════════════════════════════════════════════════════════════════ */

void ball_pid_set_q3(void)
{
    pos_pid.Kp = Q3_POS_KP; pos_pid.Ki = Q3_POS_KI; pos_pid.Kd = Q3_POS_KD;
    pos_pid.err_sum = 0.0f; pos_pid.last_err = 0.0f;
    vel_pid.Kp = Q3_VEL_KP; vel_pid.Ki = Q3_VEL_KI; vel_pid.Kd = Q3_VEL_KD;
    vel_pid.err_sum = 0.0f; vel_pid.last_err = 0.0f;
    kf = Q3_ACC_KF;
}

void ball_pid_set_q4(void)
{
    pos_pid.Kp = Q4_POS_KP; pos_pid.Ki = Q4_POS_KI; pos_pid.Kd = Q4_POS_KD;
    pos_pid.err_sum = 0.0f; pos_pid.last_err = 0.0f;
    vel_pid.Kp = Q4_VEL_KP; vel_pid.Ki = Q4_VEL_KI; vel_pid.Kd = Q4_VEL_KD;
    vel_pid.err_sum = 0.0f; vel_pid.last_err = 0.0f;
    kf = Q4_ACC_KF;
}

void ball_pid_set_q5(void)
{
    pos_pid.Kp = Q5_POS_KP; pos_pid.Ki = Q5_POS_KI; pos_pid.Kd = Q5_POS_KD;
    pos_pid.err_sum = 0.0f; pos_pid.last_err = 0.0f;
    vel_pid.Kp = Q5_VEL_KP; vel_pid.Ki = Q5_VEL_KI; vel_pid.Kd = Q5_VEL_KD;
    vel_pid.err_sum = 0.0f; vel_pid.last_err = 0.0f;
    kf = Q5_ACC_KF;
}

void ball_pid_set_static(void)
{
    pos_pid.Kp = STATIC_POS_KP;
    pos_pid.Ki = STATIC_POS_KI;
    pos_pid.Kd = STATIC_POS_KD;
    pos_pid.err_sum = 0.0f;
    pos_pid.last_err = 0.0f;
    vel_pid.Kp = STATIC_VEL_KP;
    vel_pid.Ki = STATIC_VEL_KI;
    vel_pid.Kd = STATIC_VEL_KD;
    vel_pid.err_sum = 0.0f;
    vel_pid.last_err = 0.0f;
}

/* ═══════════════════════════════════════════════════════════════════
 * 外环 PID: 位置偏差(mm) → 目标球速(mm/s)
 * ═══════════════════════════════════════════════════════════════════ */

static float pos_pid_calc(float error)
{
    // if (fabsf(error) <= BALL_DEADBAND) {
    //     pos_pid.last_err = 0.0f;
    //     pos_pid.err_sum  = 0.0f;
    //     return 0.0f;
    // }

    /* 误差钳位, 防止大偏差产生过大目标速度 */
    // if (error > 30.0f)  error = 30.0f;
    // if (error < -30.0f) error = -30.0f;

    pos_pid.err      = error;
    pos_pid.err_sum  += pos_pid.err;
    pos_pid.err_diff  = pos_pid.err - pos_pid.last_err;

    if (pos_pid.err_sum > POS_INTEGRAL_MAX)  pos_pid.err_sum = POS_INTEGRAL_MAX;
    if (pos_pid.err_sum < POS_INTEGRAL_MIN)  pos_pid.err_sum = POS_INTEGRAL_MIN;

    float out = pos_pid.Kp * pos_pid.err
              + pos_pid.Ki * pos_pid.err_sum
              + pos_pid.Kd * pos_pid.err_diff;

    if (out > POS_OUT_MAX)  out = POS_OUT_MAX;
    if (out < POS_OUT_MIN)  out = POS_OUT_MIN;

    pos_pid.last_err = pos_pid.err;
    return out;
}

/* ═══════════════════════════════════════════════════════════════════
 * 内环 PID: 球速偏差(mm/s) → 水管角度(°)
 * ═══════════════════════════════════════════════════════════════════ */

static float vel_pid_calc(float error)
{
    vel_pid.err      = error;
    vel_pid.err_sum  += vel_pid.err;
    vel_pid.err_diff  = vel_pid.err - vel_pid.last_err;

    if (vel_pid.err_sum > VEL_INTEGRAL_MAX)  vel_pid.err_sum = VEL_INTEGRAL_MAX;
    if (vel_pid.err_sum < VEL_INTEGRAL_MIN)  vel_pid.err_sum = VEL_INTEGRAL_MIN;

    float out = vel_pid.Kp * vel_pid.err
              + vel_pid.Ki * vel_pid.err_sum
              + vel_pid.Kd * vel_pid.err_diff;

    vel_pid.last_err = vel_pid.err;
    return out;
}

/* ═══════════════════════════════════════════════════════════════════
 * 串级PID 主控制 (每 10ms 调用)
 * ═══════════════════════════════════════════════════════════════════ */

void ball_balance_control(void)
{
    if (sys_tick_ms - last_k230_time > K230_TIMEOUT_MS) {
        Emm_V5_Stop_Now(STEPPER_ADDR, false);
        pos_pid.err_sum = 0.0f;
        vel_pid.err_sum = 0.0f;
        return;
    }

    /* ══════ 外环: 位置偏差(相对目标) → 目标球速 ══════ */
    float target_vel = pos_pid_calc(pos_error);

    // if(target_vel >= 100.0f){
    //     target_vel =100;
    // }
    // if(target_vel <= -100.0f){
    //     target_vel =-100;
    // }  
    target_vel_debug = target_vel;

    /* ══════ 内环: 球速偏差 → 水管角度 ══════ */
    float vel_err =ball_velocity - target_vel;
    float angle = vel_pid_calc(vel_err);
    ball_target_angle_debug = angle;

    /* ══════ 角度限位 ══════ */
    if (angle > ANGLE_MAX)  angle = ANGLE_MAX;
    if (angle < ANGLE_MIN)  angle = ANGLE_MIN;

    /* ══════ 角度 → 脉冲 → 绝对位置指令 ══════ */
    int32_t pulses;
    if(acc_flag == 0){
       pulses = (int32_t)(fabsf(angle) * PULSES_PER_DEGREE + 0.5f);
    }else{
        pulses = (int32_t)(fabsf(angle) * PULSES_PER_DEGREE + 0.5f)+kf * acc;
        // pulses = kf * acc;
        if(pulses < -220){
            pulses = -220;
        }
        if(pulses > 220 ){
            pulses = 220;
        }
    }
    
    uint8_t dir = (angle >= 0.0f) ? 0 : 1;   /* 正=CW, 负=CCW */

    Emm_V5_Pos_Control(STEPPER_ADDR, dir, STEPPER_POS_VEL, STEPPER_POS_ACC,
                       (uint32_t)pulses, 1, false);  /* raF=1 绝对位置 */
}

/* ═══════════════════════════════════════════════════════════════════
 * 纯速度环控制 (调内环PID用)
 * 输入目标球速, 跳过外环位置PID, 直接速度→角度→电机
 * ═══════════════════════════════════════════════════════════════════ */

void ball_vel_control(float target_vel)
{
    target_vel_debug = target_vel;
    if (sys_tick_ms - last_k230_time > K230_TIMEOUT_MS) {
        Emm_V5_Stop_Now(STEPPER_ADDR, false);
        vel_pid.err_sum = 0.0f;
        return;
    }

    /* 速度偏差 → 角度 */
    float vel_err = ball_velocity - target_vel;
    float angle = vel_pid_calc(vel_err);
    ball_target_angle_debug = angle;

    /* 角度限位 */
    if (angle > ANGLE_MAX)  angle = ANGLE_MAX;
    if (angle < ANGLE_MIN)  angle = ANGLE_MIN;

    /* 角度 → 脉冲 → 电机 */
    int32_t pulses = (int32_t)(fabsf(angle) * PULSES_PER_DEGREE + 0.5f);
    uint8_t dir = (angle >= 0.0f) ? 0 : 1;

    Emm_V5_Pos_Control(STEPPER_ADDR, dir, STEPPER_POS_VEL, STEPPER_POS_ACC,
                       (uint32_t)pulses, 1, false);
}
