#ifndef DRIVERS_SYSTICK_H
#define DRIVERS_SYSTICK_H

#include <stdint.h>

void systick_init(void);
uint32_t millis(void);
void delay_ms(uint32_t ms);

#endif
