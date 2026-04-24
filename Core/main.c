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

    /*
     * 初始化顺序：
     * 1. GPIO/时钟基础设施
     * 2. PWM 和 TB6612 输出
     * 3. 编码器输入和 PS2 输入
     * 4. 控制器状态
     */
    gpio_clock_enable();
    systick_init();
    pwm_init();
    tb6612_init();
    encoder_init();
    ps2_init();
    control_init();

    last_control_tick = millis();
    while (1) {
        /* 固定 10ms 控制周期，不使用 delay_ms，避免循环抖动累积。 */
        if ((millis() - last_control_tick) >= CONTROL_PERIOD_MS) {
            last_control_tick += CONTROL_PERIOD_MS;
            control_update_10ms();
        }
    }
}
