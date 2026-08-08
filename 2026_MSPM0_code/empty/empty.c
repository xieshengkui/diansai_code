#include "ti_msp_dl_config.h"
#include "headfile.h"

// float    target_speed = 50.0f;
extern float target_vel_debug;
extern float ball_velocity ;

volatile uint32_t sys_tick_ms = 0;  
volatile uint32_t lap_start_tick = 0; 
volatile uint32_t lap_stop_tick = 0;      // 计时结束时刻
uint32_t last_k230_time = 0;              // 最后一次收到 K230 数据的时间

uint16_t task[6] = {10, 10, 200, 10, 50,10};  /* task[2]=200ms显示, task[3]=10ms按键 */
float    ypr[3];
volatile float    current_speed_left;
volatile float    current_speed_right;
volatile float    last_speed_left;
volatile float    last_speed_right;
volatile float    acc;

char     tx_buf[100];

char     k230_cmd;
int32_t  k230_value;
int32_t  k230_pos = 0;
float    pos_error = 0.0f;

int8_t flag1 = 0;
uint8_t flag2 = 0;


extern float q5_target;


extern float dist;

void TIMER_0_Init(void)
{
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
}

int main(void)
{
    delay_ms(1000);
    SYSCFG_DL_init();
    TIMER_0_Init();
    motor_init();
    encoder_init();
    bluetooth_init();
    k230_init();
    stepper_init();
    //IMU_init();
    OLED_Init();
    question_task_init();
    key_init();
    menu_init();
    menu_display();
    delay_ms(100);
    Emm_V5_En_Control(1, true, false);
    delay_ms(100);
    Emm_V5_Pos_Control(1, 0, 300, 100, 0, 1, false);
    delay_ms(100);
    while (1) {
        if (Command_GetResult(&k230_cmd, &k230_value)) {
                if (k230_cmd == 'X') {
                    k230_pos = k230_value;         /* 位置单独存, 不被 V 覆盖 */
                    last_k230_time = sys_tick_ms;
                }
                if (k230_cmd == 'V') {
                    // /* 一阶低通滤波，滤波系数 0.5 */
                    static float ball_velocity_filtered = 0.0f;
                    // ball_velocity_filtered += 0.5f * ((float)k230_value - ball_velocity_filtered);
                    // ball_velocity = ball_velocity_filtered;  /* 小球实际速度 mm/s */
                    ball_velocity_filtered = 0.7 * k230_value + 0.3 * ball_velocity_filtered;
                    ball_velocity = ball_velocity_filtered;
                }
            }


        /* ── task[0]: 主控制 (10ms) ── */
        if (!task[0]) {
            task[0] = 10;

            switch (STATE_MACHINE.Main_State) {
            case STOP_STATE:
                speed_control(0.0f, 0.0f);
                break;
            case Q1_STATE:
                question1_task();
                break;
            // case Q2_STATE:
            //     question2_task();
            //     break;
            case Q3_STATE:
                question3_task();
                break;
            case Q4_STATE:
                question4_task();
                break;
            case Q5_STATE:
                question5_task();
                break;
            default:
                break;
            }

            last_speed_left = current_speed_left;
            last_speed_right = current_speed_right;
            current_speed_left  = encoder_get_speed_left();
            current_speed_right = encoder_get_speed_right();
            acc = ((current_speed_left+current_speed_right)-(last_speed_left+last_speed_right))/2/0.01;
        }

        /* ── task[1]: 球平衡 (30ms) ── */
        if (!task[1]) {
            task[1] = 10;

            if (STATE_MACHINE.Main_State == Q2_STATE) {
                question2_task();
            } else if (STATE_MACHINE.Main_State == Q5_STATE
                       && STATE_MACHINE.Q5_State == Q5_State_3) {
                pos_error = q5_target - k230_pos;
                ball_balance_control();
            } else if (STATE_MACHINE.Main_State == STOP_STATE
                       || STATE_MACHINE.Main_State == Q1_STATE
                       || STATE_MACHINE.Main_State == Q3_STATE
                       || STATE_MACHINE.Main_State == Q4_STATE) {
                pos_error =  - k230_pos;
                ball_balance_control();
            }
        }

        /* ── task[2]: OLED显示刷新 (200ms) ── */
        if (!task[2]) {
            task[2] = 200;
            menu_display_process();
        }

        /* ── task[3]: 按键扫描 (10ms) ── */
        if (!task[3]) {
            task[3] = 10;
            menu_key_process();
        }

        /* ── task[4]: 蓝牙数据发送 (50ms) ── */
        if (!task[4]) {
            task[4] = 50;
            float track_error = track_get_error();
            sprintf(tx_buf,"[plot,%.2f]\r\n" ,track_error);
            bluetooth_write((uint8_t*)tx_buf,strlen(tx_buf));
        }
    }
}


/* ── 重定向fputc ── */
int fputc(int ch, FILE *stream)
{
    static uint8_t byte;
    byte = (uint8_t)ch;
    bluetooth_write(&byte, 1);
    return ch;
}

/* ── 重定向fputs ── */
int fputs(const char *restrict s, FILE *restrict stream)
{
    uint16_t char_len = strlen(s);
    bluetooth_write((uint8_t *)s, char_len);
    return char_len;
}

int puts(const char *_ptr)
{
    return 0;
}

/* ── 1ms定时器中断：递减task[] ── */
void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(TIMER_0_INST)) {
    case DL_TIMER_IIDX_ZERO:
        
        sys_tick_ms++;
        for (int i = 0; i < 6; i++) {
            if (task[i] > 0) {
                task[i]--;
            }
        }
        break;
    default:
        break;
    }
}
