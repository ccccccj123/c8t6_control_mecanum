#include "config.h"
#include "control.h"
#include "encoder.h"
#include "gpio.h"
#include "ps2.h"
#include "pwm.h"
#include "systick.h"
#include "tb6612.h"

int main(void) {
    uint32_t last_control_tick;

    gpio_clock_enable();
    systick_init();
    pwm_init();
    tb6612_init();
    encoder_init();
    ps2_init();
    control_init();

    last_control_tick = millis();
    while (1) {
        if ((millis() - last_control_tick) >= CONTROL_PERIOD_MS) {
            last_control_tick += CONTROL_PERIOD_MS;
            control_update_10ms();
        }
    }
}
