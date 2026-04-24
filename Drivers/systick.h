#ifndef DRIVERS_SYSTICK_H
#define DRIVERS_SYSTICK_H

#include <stdint.h>

/* 初始化 Cortex-M3 SysTick，每 1ms 进入一次 SysTick_Handler。 */
void systick_init(void);

/* 返回上电后的毫秒计数，32 位溢出对差值计算是安全的。 */
uint32_t millis(void);

/* 简单阻塞延时，主要用于初始化或调试，不建议放进高速控制路径。 */
void delay_ms(uint32_t ms);

#endif
