#include "encoder.h"
#include "gpio.h"
#include "stm32f103xb.h"

static volatile int32_t rr_count;
static uint8_t rr_last_state;
static int16_t last_timer_count[3];

/*
 * 当前定时器资源分配：
 * - TIM1: PA8-PA11 四路硬件 PWM，给四个 TB6612 PWM 输入。
 * - TIM2: PA0/PA1，前左轮硬件编码器。
 * - TIM3: PA6/PA7，前右轮硬件编码器。
 * - TIM4: PB6/PB7，后左轮硬件编码器。
 * - 后右轮 PA4/PA5 使用 EXTI 软件正交解码。
 *
 * 第四路“可以”用硬件编码器吗？
 * 可以，但需要把 TIM1 改成编码器模式，典型脚位是 PA8/PA9。
 * 问题是 TIM1 同时是本工程唯一完整保留的四路硬件 PWM 来源。
 * 如果 TIM1 拿去做第四路编码器，就必须把电机 PWM 改成软件 PWM、
 * 低频定时中断 PWM，或者外接 PCA9685/TLC5940 一类 PWM 芯片。
 * 对 TB6612 电机控制来说，稳定 20kHz 硬件 PWM 比第四路硬件解码更重要，
 * 所以这里选择“三路硬件编码器 + 一路 EXTI 软件编码器”。
 */

/* TIM2/TIM3/TIM4 使用编码器模式 3：A/B 两相都参与计数。 */
static void encoder_timer_init(TIM_TypeDef *tim) {
    tim->PSC = 0U;
    tim->ARR = 0xFFFFU;
    tim->CNT = 0U;
    /*
     * CCMR1:
     * CC1S=01，把 CH1 配成输入并映射到 TI1；
     * CC2S=01，把 CH2 配成输入并映射到 TI2。
     * 这里没有加输入滤波，如果实车编码器线较长、抖动大，可以在这里加入 ICxF。
     */
    tim->CCMR1 = (1UL << 0) | (1UL << 8);
    tim->CCER = 0U;
    /* SMS=011：编码器模式 3，上升/下降沿都会根据另一相电平改变计数方向。 */
    tim->SMCR = 3U;
    tim->CR1 = TIM_CR1_CEN;
}

static int8_t quadrature_delta(uint8_t previous, uint8_t current) {
    /*
     * 四倍频正交解码表。
     *
     * AB 相正常转动时状态按 00->01->11->10->00 或反方向循环。
     * previous/current 均为 2bit AB 状态，因此可以拼成 4bit 索引。
     * 表中 +1/-1 表示有效一步，0 表示没动或非法跳变。
     * 非法跳变返回 0，可以过滤一部分毛刺，但如果编码器线噪声很大，
     * 仍然建议从硬件上加屏蔽线、短线、上拉或小电容滤波。
     */
    static const int8_t table[16] = {
        0, 1, -1, 0,
        -1, 0, 0, 1,
        1, 0, 0, -1,
        0, -1, 1, 0
    };
    return table[((previous & 0x3U) << 2) | (current & 0x3U)];
}

static uint8_t rr_state(void) {
    /* 后右轮使用 PA4/PA5 做软件解码，bit1=A, bit0=B。 */
    return (uint8_t)((gpio_read(GPIOA, 4) << 1) | gpio_read(GPIOA, 5));
}

void encoder_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;

    /*
     * 编码器输入全部开上拉，适配开漏/集电极输出的霍尔编码器板。
     * 如果你的霍尔编码器模块已经有强上拉，内部上拉不会造成问题；
     * 如果模块是 5V 推挽输出，不要直接接 STM32，先确认电平安全。
     */
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

    /* 前左 TIM2=PA0/PA1，前右 TIM3=PA6/PA7，后左 TIM4=PB6/PB7。 */
    encoder_timer_init(TIM2);
    encoder_timer_init(TIM3);
    encoder_timer_init(TIM4);

    /*
     * PA4/PA5 做后右轮软件解码。
     * 两相都开上升沿和下降沿中断，达到软件四倍频计数。
     * 370 霍尔电机编码器频率通常不高，C8T6 在 72MHz 下处理一路 EXTI 足够。
     */
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
        /* int16_t 差值天然处理 16 位定时器溢出，适合周期性读取。 */
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
