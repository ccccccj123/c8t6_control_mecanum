#include "stm32f103xb.h"
#include "config.h"

uint32_t SystemCoreClock = SYSCLK_HZ;

static void clock_init_hse_72mhz(void) {
    /* Blue Pill/最小系统板通常外接 8MHz 晶振，先等待 HSE 稳定。 */
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0U) {
    }

    /* 72MHz 需要 Flash 两个等待周期，并打开预取缓冲。 */
    FLASH_ACR = 0x12U;

    /* AHB=72MHz，APB1=36MHz，APB2=72MHz，PLL=HSE*9。 */
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 |
                RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PLLSRC_HSE |
                RCC_CFGR_PLLMULL9;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
    }

    /* 切换系统时钟到 PLL 输出。 */
    RCC->CFGR = (RCC->CFGR & ~0x3U) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & (3UL << 2)) != RCC_CFGR_SWS_PLL) {
    }
}

void SystemInit(void) {
    /* Keil 启动文件在进入 main() 前调用 SystemInit()。 */
    clock_init_hse_72mhz();
    SystemCoreClock = SYSCLK_HZ;
}
