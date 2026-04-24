#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <stdint.h>
#include "stm32f103xb.h"

/* 打开 AFIO/GPIOA/GPIOB/GPIOC 时钟，所有引脚配置前必须先调用。 */
void gpio_clock_enable(void);

/* 配置单个 GPIO 引脚的 MODE/CNF，参数值定义在 stm32f103xb.h。 */
void gpio_config(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t cnf);

/* 写单个输出脚，使用 BSRR/BRR 原子置位/复位。 */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);

/* 读单个输入脚，返回 0 或 1。 */
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif
