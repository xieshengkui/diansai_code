#include "ti_msp_dl_config.h"
#include "headfile.h"
uint8_t uart_data;
char cmd_char;
int number;
int x_error;
int y_error;

uint16_t task[4]={10,0,0,0};
char bt_buf[32];

volatile uint32_t sys_tick_ms = 0;
uint32_t last_k230_time = 0;

void TIMER_0_Init(void)
{
	NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
}

void stepper_Init(void)
{
	DL_DMA_enableChannel(DMA, STEPPER_TX_CHAN_ID);
    DL_DMA_enableChannel(DMA, STEPPER_RX_CHAN_ID);
}

void stepper_write(uint8_t *data, uint16_t size)
{
    DL_DMA_setSrcAddr(DMA, STEPPER_TX_CHAN_ID, (uint32_t)(data));
    DL_DMA_setDestAddr(DMA, STEPPER_TX_CHAN_ID, (uint32_t)(&STEPPER_INST->TXDATA));
    DL_DMA_setTransferSize(DMA, STEPPER_TX_CHAN_ID, size);
    DL_DMA_enableChannel(DMA, STEPPER_TX_CHAN_ID);
}

void bluetooth_write(uint8_t *data, uint16_t size)
{
    DL_DMA_setSrcAddr(DMA, BT_TX_CHAN_ID, (uint32_t)(data));
    DL_DMA_setDestAddr(DMA, BT_TX_CHAN_ID, (uint32_t)(&BT_INST->TXDATA));
    DL_DMA_setTransferSize(DMA, BT_TX_CHAN_ID, size);
    DL_DMA_enableChannel(DMA, BT_TX_CHAN_ID);
}

void K230_Init(){
    NVIC_ClearPendingIRQ(K230_INST_INT_IRQN);
    NVIC_EnableIRQ(K230_INST_INT_IRQN);
    DL_UART_enableInterrupt(K230_INST, DL_UART_INTERRUPT_RX);
}

int main(void)
{
    SYSCFG_DL_init();
    TIMER_0_Init();
    stepper_Init();
    K230_Init();
    OLED_Init();
    delay_ms(100);
    Emm_V5_En_Control(1, true, false);
    delay_ms(100);
    Emm_V5_En_Control(2, true, false);
    delay_ms(100);
    Emm_V5_Reset_CurPos_To_Zero(1);
    delay_ms(100);
    Emm_V5_Reset_CurPos_To_Zero(2);
    delay_ms(100);
    while (1) {
        uint8_t command_res = Command_GetResult(&cmd_char, &number);
        if(command_res == 1){
            last_k230_time = sys_tick_ms;
            if (cmd_char == 'X') {
                x_error = number;
            }else if (cmd_char == 'Y') {
                y_error = number;
            }
        }
        if(task[0]==0){
            task[0]=10;
            follow_control();
            OLED_ShowSignedNum(0, 0, x_error, 4, 16, 1);
            OLED_ShowSignedNum(0, 16, y_error, 4, 16, 1);
            OLED_Refresh();
        }
    }
}

int fputc(int ch, FILE *stream)
{
    while( DL_UART_isBusy(STEPPER_INST) == true );
    DL_UART_Main_transmitData(STEPPER_INST, ch);
    return ch;
}

//重定向fputs函数
int fputs(const char* restrict s, FILE* restrict stream) {

    uint16_t char_len=0;
    while(*s!=0)
    {
        while( DL_UART_isBusy(STEPPER_INST) == true );
        DL_UART_Main_transmitData(STEPPER_INST, *s++);
        char_len++;
    }
    return char_len;
}
int puts(const char* _ptr)
{
 return 0;
}

void K230_INST_IRQHandler(void)
{
    switch( DL_UART_getPendingInterrupt(K230_INST) )
    {
        case DL_UART_IIDX_RX:
            uart_data = DL_UART_Main_receiveData(K230_INST);
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
            sys_tick_ms++;
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