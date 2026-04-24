#include "gpio.h"

void gpio_clock_enable(void) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
}

void gpio_config(GPIO_TypeDef *port, uint8_t pin, uint8_t mode, uint8_t cnf) {
    uint32_t shift = (uint32_t)(pin & 0x7U) * 4U;
    uint32_t value = ((uint32_t)cnf << 2) | mode;
    __IO uint32_t *reg = (pin < 8U) ? &port->CRL : &port->CRH;
    *reg = (*reg & ~(0xFUL << shift)) | (value << shift);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value) {
    if (value != 0U) {
        port->BSRR = 1UL << pin;
    } else {
        port->BRR = 1UL << pin;
    }
}

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin) {
    return (uint8_t)((port->IDR >> pin) & 0x1U);
}
