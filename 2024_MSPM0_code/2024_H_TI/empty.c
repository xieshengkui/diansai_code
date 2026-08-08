#include "ti_msp_dl_config.h"
#include"headfile.h"


float ypr[3];
uint16_t task[4]={10,20,10,0};
double current_speed_left;
double current_speed_right;
uint8_t uart_data;

uint16_t target_speed=150;

extern volatile int32_t Encoder_Count_L;
extern volatile int32_t Encoder_Count_R;

void delay_ms(uint32_t __ms);


void TIMER_0_Init(void)
{
	NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
}

void bluetooth_init(){
    NVIC_ClearPendingIRQ(SCREEN_INST_INT_IRQN);  // 清除挂起中断
    NVIC_EnableIRQ(SCREEN_INST_INT_IRQN);        // 开启串口总中断
    DL_UART_enableInterrupt(SCREEN_INST, DL_UART_INTERRUPT_RX); 
}

void bluetooth_write(uint8_t *data, uint16_t size)
{
    DL_DMA_setSrcAddr(DMA, bluetooth_DMA_CHAN_ID, (uint32_t)(data));
    DL_DMA_setDestAddr(DMA, bluetooth_DMA_CHAN_ID, (uint32_t)(&BT_INST->TXDATA));
    DL_DMA_setTransferSize(DMA, bluetooth_DMA_CHAN_ID, size);
    DL_DMA_enableChannel(DMA, bluetooth_DMA_CHAN_ID);
}

char tx_buf[100];
char command;
int number;

int main(void)
{
    SYSCFG_DL_init();
    TIMER_0_Init();
    motor_init();
    encoder_init();
    question_task_init();
    bluetooth_init();
    DL_DMA_enableChannel(DMA, bluetooth_DMA_CHAN_ID);
    IMU_init();
    delay_ms(100);
    
    
    while (1)
    {
    uint8_t command_res = Command_GetResult(&command, &number);
    if(command_res != 0){
        if(command == 'T' && number == 1){
            STATE_MACHINE.Main_State = Q1_STATE;
            q1_first_flag = 0;
        }else if(command == 'T' && number == 2){
            STATE_MACHINE.Main_State = Q2_STATE;
            q2_first_flag = 0;
        }else if(command == 'T' && number == 3){
            STATE_MACHINE.Main_State = Q3_STATE;
            q3_first_flag = 0;
        }else if(command == 'T' && number == 4){
            STATE_MACHINE.Main_State = Q4_STATE;
            q4_first_flag = 0;
        }else if (command == 'V') {
            target_speed = number;
            sprintf(tx_buf,"%c,%d\r\n",command,number);
            bluetooth_write((uint8_t*)tx_buf,strlen(tx_buf));
        }
    }
    
    if(!task[0]){
      task[0] = 10;
      buzzer_control();
      switch(STATE_MACHINE.Main_State) {
        case STOP_STATE:
            speed_control(0.0f, 0.0f);
            break;
        case Q1_STATE:
            question1_task();
            break;
        case Q2_STATE:
            question2_task();
            break;
        case Q3_STATE:
            question3_task();
            break;
        case Q4_STATE:
            question4_task();
            break;
        default:
            break;
      }
      current_speed_left  = encoder_get_speed_left();
      current_speed_right = encoder_get_speed_right();
      sprintf(tx_buf,"[plot,%.2f,%.2f,%.2f]\r\n",current_speed_left,current_speed_right,ypr[0]);
      bluetooth_write((uint8_t*)tx_buf,strlen(tx_buf));
    }
    if(!task[1]){
      task[1] = 20;
      IMU_getYawPitchRoll(ypr);
    }
    }
}

//重定向fputc函数
int fputc(int ch, FILE *stream)
{
    while( DL_UART_isBusy(BT_INST) == true );
    DL_UART_Main_transmitData(BT_INST, ch);
    return ch;
}

//重定向fputs函数
int fputs(const char* restrict s, FILE* restrict stream) {

    uint16_t char_len=0;
    while(*s!=0)
    {
        while( DL_UART_isBusy(BT_INST) == true );
        DL_UART_Main_transmitData(BT_INST, *s++);
        char_len++;
    }
    return char_len;
}
int puts(const char* _ptr)
{
 return 0;
}

//串口的中断服务函数
void SCREEN_INST_IRQHandler(void)
{
    switch( DL_UART_getPendingInterrupt(SCREEN_INST) )
    {
        case DL_UART_IIDX_RX:
            uart_data = DL_UART_Main_receiveData(SCREEN_INST);
            Command_Write(&uart_data, 1);
            break;

        default://其他的串口中断
            break;
    }
}


void TIMER_0_INST_IRQHandler(void)
{
	switch(DL_Timer_getPendingInterrupt(TIMER_0_INST))
	{
		case DL_TIMER_IIDX_ZERO:
            for(int i=0; i<4; i++) {
                if(task[i] > 0) {
                    task[i]--;
                }
      }
		break;
		default:
			
		break;
	}
}
