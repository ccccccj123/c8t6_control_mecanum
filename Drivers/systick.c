#include "systick.h"
#include "stm32f103xb.h"
#include "config.h"

static volatile uint32_t g_millis;

void systick_init(void) {
    SYST_RVR = (SYSCLK_HZ / 1000UL) - 1UL;
    SYST_CVR = 0U;
    SYST_CSR = 0x7U;
}

uint32_t millis(void) {
    return g_millis;
}

void delay_ms(uint32_t ms) {
    uint32_t start = millis();
    while ((millis() - start) < ms) {
    }
}

void SysTick_Handler(void) {
    g_millis++;
}
