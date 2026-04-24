#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <stdint.h>
#include "stm32f103xb.h"

void gpio_clock_enable(void);
void gpio_config(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t cnf);
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif
