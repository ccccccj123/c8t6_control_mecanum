#include "stm32f103xb.h"
#include "config.h"

uint32_t SystemCoreClock = SYSCLK_HZ;

static void clock_init_hse_72mhz(void) {
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0U) {
    }

    FLASH_ACR = 0x12U;
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 |
                RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PLLSRC_HSE |
                RCC_CFGR_PLLMULL9;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
    }

    RCC->CFGR = (RCC->CFGR & ~0x3U) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & (3UL << 2)) != RCC_CFGR_SWS_PLL) {
    }
}

void SystemInit(void) {
    clock_init_hse_72mhz();
    SystemCoreClock = SYSCLK_HZ;
}
