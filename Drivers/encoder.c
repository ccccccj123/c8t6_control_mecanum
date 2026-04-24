#include "encoder.h"
#include "gpio.h"
#include "stm32f103xb.h"

static volatile int32_t rr_count;
static uint8_t rr_last_state;
static int16_t last_timer_count[3];

static void encoder_timer_init(TIM_TypeDef *tim) {
    tim->PSC = 0U;
    tim->ARR = 0xFFFFU;
    tim->CNT = 0U;
    tim->CCMR1 = (1UL << 0) | (1UL << 8);
    tim->CCER = 0U;
    tim->SMCR = 3U;
    tim->CR1 = TIM_CR1_CEN;
}

static int8_t quadrature_delta(uint8_t previous, uint8_t current) {
    static const int8_t table[16] = {
        0, 1, -1, 0,
        -1, 0, 0, 1,
        1, 0, 0, -1,
        0, -1, 1, 0
    };
    return table[((previous & 0x3U) << 2) | (current & 0x3U)];
}

static uint8_t rr_state(void) {
    return (uint8_t)((gpio_read(GPIOA, 4) << 1) | gpio_read(GPIOA, 5));
}

void encoder_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;

    gpio_config(GPIOA, 0, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOA, 1, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOA, 6, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOA, 7, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOB, 6, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOB, 7, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOA, 4, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    gpio_config(GPIOA, 5, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    GPIOA->ODR |= (1UL << 0) | (1UL << 1) | (1UL << 4) | (1UL << 5) |
                  (1UL << 6) | (1UL << 7);
    GPIOB->ODR |= (1UL << 6) | (1UL << 7);

    encoder_timer_init(TIM2);
    encoder_timer_init(TIM3);
    encoder_timer_init(TIM4);

    rr_last_state = rr_state();
    AFIO->EXTICR[1] &= ~((0xFUL << 0) | (0xFUL << 4));
    EXTI->IMR |= (1UL << 4) | (1UL << 5);
    EXTI->RTSR |= (1UL << 4) | (1UL << 5);
    EXTI->FTSR |= (1UL << 4) | (1UL << 5);
    NVIC_ISER0 = (1UL << EXTI4_IRQn) | (1UL << EXTI9_5_IRQn);
}

int16_t encoder_read_delta(MotorId motor) {
    int16_t now;
    int16_t delta = 0;

    switch (motor) {
    case MOTOR_FRONT_LEFT:
        now = (int16_t)TIM2->CNT;
        delta = (int16_t)(now - last_timer_count[0]);
        last_timer_count[0] = now;
        break;
    case MOTOR_FRONT_RIGHT:
        now = (int16_t)TIM3->CNT;
        delta = (int16_t)(now - last_timer_count[1]);
        last_timer_count[1] = now;
        break;
    case MOTOR_REAR_LEFT:
        now = (int16_t)TIM4->CNT;
        delta = (int16_t)(now - last_timer_count[2]);
        last_timer_count[2] = now;
        break;
    case MOTOR_REAR_RIGHT:
        delta = (int16_t)rr_count;
        rr_count = 0;
        break;
    default:
        break;
    }
    return delta;
}

void EXTI4_IRQHandler(void) {
    uint8_t current = rr_state();
    rr_count += quadrature_delta(rr_last_state, current);
    rr_last_state = current;
    EXTI->PR = 1UL << 4;
}

void EXTI9_5_IRQHandler(void) {
    uint8_t current = rr_state();
    rr_count += quadrature_delta(rr_last_state, current);
    rr_last_state = current;
    EXTI->PR = 1UL << 5;
}
