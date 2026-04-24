#include "tb6612.h"
#include "gpio.h"
#include "pwm.h"
#include "config.h"

typedef struct {
    GPIO_TypeDef *in1_port;
    uint8_t in1_pin;
    GPIO_TypeDef *in2_port;
    uint8_t in2_pin;
    uint8_t pwm_channel;
} MotorPinMap;

static const MotorPinMap motor_pins[MOTOR_COUNT] = {
    {GPIOB, 0, GPIOB, 1, 1},
    {GPIOB, 10, GPIOB, 11, 2},
    {GPIOA, 12, GPIOA, 15, 3},
    {GPIOC, 13, GPIOC, 14, 4},
};

static uint16_t magnitude(int16_t value) {
    return (uint16_t)(value < 0 ? -value : value);
}

void tb6612_init(void) {
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        gpio_config(motor_pins[i].in1_port, motor_pins[i].in1_pin,
                    GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
        gpio_config(motor_pins[i].in2_port, motor_pins[i].in2_pin,
                    GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
    }
    gpio_config(GPIOB, 2, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
    tb6612_set_standby(1U);
    tb6612_stop_all();
}

void tb6612_set_standby(uint8_t enabled) {
    gpio_write(GPIOB, 2, enabled);
}

void tb6612_set_motor(MotorId motor, int16_t command) {
    const MotorPinMap *pins;
    uint16_t duty;

    if (motor >= MOTOR_COUNT) {
        return;
    }

    pins = &motor_pins[motor];
    duty = magnitude(command);
    if (duty > PWM_MAX_DUTY) {
        duty = PWM_MAX_DUTY;
    }

    if (command > 0) {
        gpio_write(pins->in1_port, pins->in1_pin, 1U);
        gpio_write(pins->in2_port, pins->in2_pin, 0U);
    } else if (command < 0) {
        gpio_write(pins->in1_port, pins->in1_pin, 0U);
        gpio_write(pins->in2_port, pins->in2_pin, 1U);
    } else {
        gpio_write(pins->in1_port, pins->in1_pin, 0U);
        gpio_write(pins->in2_port, pins->in2_pin, 0U);
    }
    pwm_set_duty(pins->pwm_channel, duty);
}

void tb6612_stop_all(void) {
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        tb6612_set_motor((MotorId)i, 0);
    }
}
