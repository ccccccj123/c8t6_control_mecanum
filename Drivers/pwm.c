#include "pwm.h"
#include "gpio.h"
#include "stm32f103xb.h"
#include "config.h"

void pwm_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    /* PA8-PA11 是 TIM1 CH1-CH4，配置为复用推挽输出。 */
    gpio_config(GPIOA, 8, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);
    gpio_config(GPIOA, 9, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);
    gpio_config(GPIOA, 10, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);
    gpio_config(GPIOA, 11, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_AF_PP);

    /*
     * TIM1 在 APB2 上，72MHz 计数。
     *
     * 为什么用 TIM1 做 PWM：
     * - PA8/PA9/PA10/PA11 正好是 TIM1 CH1-CH4，四路都在一组定时器里；
     * - 四个电机的 PWM 频率完全同步，不会因为软件延迟出现相位乱跳；
     * - 20kHz PWM 比较安静，TB6612 也能正常接受。
     *
     * ARR = 72MHz / 20kHz - 1，占空比通过 CCRx 设置。
     */
    TIM1->PSC = 0U;
    TIM1->ARR = (SYSCLK_HZ / PWM_FREQUENCY_HZ) - 1U;
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCR4 = 0U;
    /* PWM mode 1 + 预装载，使占空比更新平滑。 */
    TIM1->CCMR1 = (6UL << 4) | (1UL << 3) | (6UL << 12) | (1UL << 11);
    TIM1->CCMR2 = (6UL << 4) | (1UL << 3) | (6UL << 12) | (1UL << 11);
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    /* 高级定时器 TIM1 必须打开 MOE，否则 PWM 引脚不会真正输出。 */
    TIM1->BDTR = TIM_BDTR_MOE;
    TIM1->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    TIM1->EGR = TIM_EGR_UG;
}

void pwm_set_duty(uint8_t channel, uint16_t duty) {
    uint32_t compare;
    if (duty > PWM_MAX_DUTY) {
        duty = PWM_MAX_DUTY;
    }
    /* 把 0-1000 的软件占空比映射到 TIM1 的实际比较值。 */
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
