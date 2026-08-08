#include "headfile.h"

void delay_us(uint32_t __us) { delay_cycles( (CPUCLK_FREQ / 1000 / 1000)*__us); }
void delay_ms(uint32_t __ms) { delay_cycles( (CPUCLK_FREQ / 1000)*__ms); }
