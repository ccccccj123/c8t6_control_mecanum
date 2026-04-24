#include "pwm.h"
#include "gpio.h"
#include "stm32f103xb.h"
#include "config.h"

void pwm_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    gpio_config(GPIOA, 8, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);
    gpio_config(GPIOA, 9, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);
    gpio_config(GPIOA, 10, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);
    gpio_config(GPIOA, 11, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);

    TIM1->PSC = 0U;
    TIM1->ARR = (SYSCLK_HZ / PWM_FREQUENCY_HZ) - 1U;
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCR4 = 0U;
    TIM1->CCMR1 = (6UL << 4) | (1UL << 3) | (6UL << 12) | (1UL << 11);
    TIM1->CCMR2 = (6UL << 4) | (1UL << 3) | (6UL << 12) | (1UL << 11);
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    TIM1->BDTR = TIM_BDTR_MOE;
    TIM1->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    TIM1->EGR = TIM_EGR_UG;
}

void pwm_set_duty(uint8_t channel, uint16_t duty) {
    uint32_t compare;
    if (duty > PWM_MAX_DUTY) {
        duty = PWM_MAX_DUTY;
    }
    compare = ((TIM1->ARR + 1U) * duty) / PWM_MAX_DUTY;

    switch (channel) {
    case 1:
        TIM1->CCR1 = compare;
        break;
    case 2:
        TIM1->CCR2 = compare;
        break;
    case 3:
        TIM1->CCR3 = compare;
        break;
    case 4:
        TIM1->CCR4 = compare;
        break;
    default:
        break;
    }
}
