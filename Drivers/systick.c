#include "systick.h"
#include "stm32f103xb.h"
#include "config.h"

static volatile uint32_t g_millis;

void systick_init(void) {
    /* LOAD = 72MHz / 1000 - 1，CTRL=时钟源处理器时钟 + 中断 + 使能。 */
    SYST_RVR = (SYSCLK_HZ / 1000UL) - 1UL;
    SYST_CVR = 0U;
    SYST_CSR = 0x7U;
}

uint32_t millis(void) {
    return g_millis;
}

void delay_ms(uint32_t ms) {
    uint32_t start = millis();
    /* 使用减法判断可以自然处理 millis() 溢出。 */
    while ((millis() - start) < ms) {
    }
}

void SysTick_Handler(void) {
    /* 1ms 全局节拍，主循环用它调度 10ms 控制周期。 */
    g_millis++;
}
