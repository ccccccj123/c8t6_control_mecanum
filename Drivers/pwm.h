#ifndef DRIVERS_PWM_H
#define DRIVERS_PWM_H

#include <stdint.h>

/* 初始化 TIM1 CH1-CH4，对应 PA8-PA11 四路电机 PWM。 */
void pwm_init(void);

/* 设置 1-4 通道占空比，duty 范围 0-PWM_MAX_DUTY。 */
void pwm_set_duty(uint8_t channel, uint16_t duty);

#endif
