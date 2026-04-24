#ifndef DRIVERS_LED_H
#define DRIVERS_LED_H

#include <stdint.h>

/*
 * C8T6 最小系统板常见板载 LED 接在 PC13。
 * 大多数板子是低电平点亮：PC13=0 亮，PC13=1 灭。
 */
void led_init(void);
void led_set(uint8_t on);
void led_toggle(void);

#endif
