#include "headfile.h"

extern int x_error;
extern volatile uint32_t sys_tick_ms;
extern uint32_t last_k230_time;


/**
  * @brief    重启电机（Y42）
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Reset_Motor(uint8_t addr)
{
  static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x08;                       // 功能码
  cmd[2] =  0x97;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
  stepper_write((uint8_t* )cmd,4);
}

/**
  * @brief    将当前位置清零
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Reset_CurPos_To_Zero(uint8_t addr)
{
  static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x0A;                       // 功能码
  cmd[2] =  0x6D;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
  stepper_write((uint8_t* )cmd,4);
}

/**
  * @brief    使能信号控制
  * @param    addr  ：电机地址
  * @param    state ：使能状态     ，true为使能电机，false为关闭电机
  * @param    snF   ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_En_Control(uint8_t addr, bool state, bool snF)
{
  __IO static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF3;                       // 功能码
  cmd[2] =  0xAB;                       // 辅助码
  cmd[3] =  (uint8_t)state;             // 使能状态
  cmd[4] =  snF;                        // 多机同步运动标志
  cmd[5] =  0x6B;                       // 校验字节
  
  // 发送命令
  stepper_write((uint8_t* )cmd,6);
}

/**
  * @brief    速度模式
  * @param    addr：电机地址
  * @param    dir ：方向       ，0为CW，其余值为CCW
  * @param    vel ：速度       ，范围0 - 5000RPM
  * @param    acc ：加速度     ，范围0 - 255，注意：0是直接启动
  * @param    snF ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Vel_Control_1(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
   static uint8_t cmd[16] = {0};

  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF6;                       // 功能码
  cmd[2] =  dir;                        // 方向
  cmd[3] =  (uint8_t)(vel >> 8);        // 速度(RPM)高8位字节
  cmd[4] =  (uint8_t)(vel >> 0);        // 速度(RPM)低8位字节
  cmd[5] =  acc;                        // 加速度，注意：0是直接启动
  cmd[6] =  snF;                        // 多机同步运动标志
  cmd[7] =  0x6B;                       // 校验字节

  // 发送命令
  stepper_write((uint8_t* )cmd,8);
}

/**
  * @brief    速度模式(独立缓冲区，与 Vel_Control 不互斥)
  * @param    addr：电机地址
  * @param    dir ：方向       ，0为CW，其余值为CCW
  * @param    vel ：速度       ，范围0 - 5000RPM
  * @param    acc ：加速度     ，范围0 - 255，注意：0是直接启动
  * @param    snF ：多机同步标志，false为不启用，true为启用
  */
void Emm_V5_Vel_Control_2(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
   static uint8_t cmd[16] = {0};        // ← 独立的 static 内存

  cmd[0] =  addr;
  cmd[1] =  0xF6;
  cmd[2] =  dir;
  cmd[3] =  (uint8_t)(vel >> 8);
  cmd[4] =  (uint8_t)(vel >> 0);
  cmd[5] =  acc;
  cmd[6] =  snF;
  cmd[7] =  0x6B;

  stepper_write((uint8_t* )cmd,8);
}

/**
  * @brief    位置模式
  * @param    addr：电机地址
  * @param    dir ：方向        ，0为CW，其余值为CCW
  * @param    vel ：速度(RPM)   ，范围0 - 5000RPM
  * @param    acc ：加速度      ，范围0 - 255，注意：0是直接启动
  * @param    clk ：脉冲数      ，范围0- (2^32 - 1)个
  * @param    raF ：运动标志，0为相对上一输入目标位置进行相对位置运动，1为绝对值运动，2相对当前电机实时位置进行相对位置运动
  * @param    snF ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, uint8_t raF, bool snF)
{
  __IO static uint8_t cmd[16] = {0};

  // 装载命令
  cmd[0]  =  addr;                      // 地址
  cmd[1]  =  0xFD;                      // 功能码
  cmd[2]  =  dir;                       // 方向
  cmd[3]  =  (uint8_t)(vel >> 8);       // 速度(RPM)高8位字节
  cmd[4]  =  (uint8_t)(vel >> 0);       // 速度(RPM)低8位字节 
  cmd[5]  =  acc;                       // 加速度，注意：0是直接启动
  cmd[6]  =  (uint8_t)(clk >> 24);      // 脉冲数(bit24 - bit31)
  cmd[7]  =  (uint8_t)(clk >> 16);      // 脉冲数(bit16 - bit23)
  cmd[8]  =  (uint8_t)(clk >> 8);       // 脉冲数(bit8  - bit15)
  cmd[9]  =  (uint8_t)(clk >> 0);       // 脉冲数(bit0  - bit7 )
  cmd[10] =  raF;                       // 相位/绝对标志，false为相对运动，true为绝对值运动
  cmd[11] =  snF;                       // 多机同步运动标志，false为不启用，true为启用
  cmd[12] =  0x6B;                      // 校验字节
  
  // 发送命令
  stepper_write((uint8_t* )cmd,13);
}

/**
  * @brief    立即停止
  * @param    addr  ：电机地址
  * @param    snF   ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Stop_Now(uint8_t addr, bool snF)
{
  __IO static uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFE;                       // 功能码
  cmd[2] =  0x98;                       // 辅助码
  cmd[3] =  snF;                        // 多机同步运动标志
  cmd[4] =  0x6B;                       // 校验字节
  
  // 发送命令
  stepper_write((uint8_t* )cmd,5);
}


/* 全局PID实例，X/Y轴各一个 */
PID stepper_pid_x = {STEPPER1_KP, STEPPER1_KI, STEPPER1_KD, 0.0f, 0.0f, 0.0f, 0.0f};
PID stepper_pid_y = {STEPPER2_KP, STEPPER2_KI, STEPPER2_KD, 0.0f, 0.0f, 0.0f, 0.0f};

/**
  * @brief  位置式PID计算 (输出 = 目标速度 RPM)
  * @param  pid      PID控制器指针
  * @param  x_error  本次像素偏差 (setpoint - measurement)
  * @retval 目标速度 (RPM)
  *
  * 公式:
  *   pout = Kp * e(k)
  *   err_sum += Ki * e(k)                     (积分累加)
  *   dout = Kd * [e(k) - e(k-1)]
  *   out  = pout + err_sum + dout
  */
float stepper1_pd_calc(PID *pid, int x_error){
    float pout, dout, out;
    /* 1. 本次误差 */
    pid->err = (float)x_error;
    /* 2. 输入死区 */
    /*if (fabsf(pid->err) <= STEPPER_DEADBAND){
        pid->err     = 0.0f;
        pid->err_sum = 0.0f;
    }*/
    /* 3. P项: Kp * e(k) */
    pout = pid->Kp * pid->err;
    /* 4. I项: 累加, 限幅 (用err_sum存积分) */
    /*pid->err_sum += pid->Ki * pid->err;
    if (pid->err_sum > STEPPER_INT_LIMIT)
        pid->err_sum = STEPPER_INT_LIMIT;
    else if (pid->err_sum < -STEPPER_INT_LIMIT)
        pid->err_sum = -STEPPER_INT_LIMIT;*/

    /* 5. D项: Kd * [e(k) - e(k-1)] (err_diff存本次误差变化量) */
    pid->err_diff = pid->err - pid->last_err;
    dout = pid->Kd * pid->err_diff;
    /* 6. 总输出, 限幅 */
    out = pout + pid->err_sum + dout;
    /*if (out > STEPPER_OUT_MAX)
        out = STEPPER_OUT_MAX;
    else if (out < -STEPPER_OUT_MAX)
        out = -STEPPER_OUT_MAX;*/
    /* 7. 误差滚动 */
    pid->last_err = pid->err;
    return out;
}

float stepper2_pd_calc(PID *pid, int x_error){
    float pout, dout, out;
    /* 1. 本次误差 */
    pid->err = (float)x_error;
    /* 2. 输入死区 */
    /*if (fabsf(pid->err) <= STEPPER_DEADBAND){
        pid->err     = 0.0f;
        pid->err_sum = 0.0f;
    }*/
    /* 3. P项: Kp * e(k) */
    pout = pid->Kp * pid->err;
    /* 4. I项: 累加, 限幅 (用err_sum存积分) */
    /*pid->err_sum += pid->Ki * pid->err;
    if (pid->err_sum > STEPPER_INT_LIMIT)
        pid->err_sum = STEPPER_INT_LIMIT;
    else if (pid->err_sum < -STEPPER_INT_LIMIT)
        pid->err_sum = -STEPPER_INT_LIMIT;*/

    /* 5. D项: Kd * [e(k) - e(k-1)] (err_diff存本次误差变化量) */
    pid->err_diff = pid->err - pid->last_err;
    dout = pid->Kd * pid->err_diff;
    /* 6. 总输出, 限幅 */
    out = pout + pid->err_sum + dout;
    /*if (out > STEPPER_OUT_MAX)
        out = STEPPER_OUT_MAX;
    else if (out < -STEPPER_OUT_MAX)
        out = -STEPPER_OUT_MAX;*/
    /* 7. 误差滚动 */
    pid->last_err = pid->err;
    return out;
}

  void follow_control(){
    float rpm_x, rpm_y;
    uint8_t dir_x, dir_y;
    uint16_t abs_x, abs_y;

    /* 数据看门狗: K230断连超过200ms → 停电机 */
    if(sys_tick_ms - last_k230_time > K230_TIMEOUT_MS){
        Emm_V5_Stop_Now(0x01, false);
        delay_cycles(800000);
        Emm_V5_Stop_Now(0x02, false);
        return;
    }

    rpm_x = stepper1_pd_calc(&stepper_pid_x,x_error);
    rpm_y = stepper2_pd_calc(&stepper_pid_y,y_error);
    if(rpm_x >= 0.0f){
      dir_x = 1;
      abs_x = (uint16_t)rpm_x;
    }else{
      dir_x = 0;
      abs_x = (uint16_t)(-rpm_x);
    }

    if(rpm_y >= 0.0f){
      dir_y = 1;
      abs_y = (uint16_t)rpm_y;
    }else{
      dir_y = 0;
      abs_y = (uint16_t)(-rpm_y);
    }
    static uint8_t toggle = 0;
    if(toggle){
      Emm_V5_Vel_Control_1(0x01, dir_x, abs_x, 0, false);   // 偶数次: 发X
    }else{
      Emm_V5_Vel_Control_2(0x02, dir_y, abs_y, 0, false);   // 奇数次: 发Y
    }
    toggle = !toggle;
  }
