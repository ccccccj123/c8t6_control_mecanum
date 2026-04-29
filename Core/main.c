#include "config.h"
#include "control.h"
#include "encoder.h"
#include "gpio.h"
#include "led.h"
#include "oled.h"
#include "ps2.h"
#include "pwm.h"
#include "systick.h"
#include "tb6612.h"

static void oled_write_int_width(int16_t value, uint8_t width) {
    char digits[7];
    uint8_t digit_count = 0U;
    uint8_t text_count;
    int32_t number = value;

    if (number < 0) {
        number = -number;
    }

    do {
        digits[digit_count++] = (char)('0' + (number % 10));
        number /= 10;
    } while (number != 0 && digit_count < sizeof(digits));

    text_count = (uint8_t)(digit_count + (value < 0 ? 1U : 0U));
    while (text_count < width) {
        oled_write_char(' ');
        text_count++;
    }

    if (value < 0) {
        oled_write_char('-');
    }
    while (digit_count > 0U) {
        oled_write_char(digits[--digit_count]);
    }
}

static void oled_write_hex_byte(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    oled_write_char(hex[(value >> 4) & 0x0FU]);
    oled_write_char(hex[value & 0x0FU]);
}

static void oled_write_ps2_mode(const ControlStatus *status) {
    if (status->ps2_connected == 0U) {
        oled_write_string("WAIT");
        return;
    }

    switch (status->ps2_mode) {
    case PS2_MODE_DIGITAL:
        oled_write_string("DIG ");
        break;
    case PS2_MODE_ANALOG_RED:
        oled_write_string("ANA ");
        break;
    case PS2_MODE_ANALOG_PRESSURE:
        oled_write_string("79  ");
        break;
    default:
        oled_write_hex_byte(status->ps2_mode);
        oled_write_string("  ");
        break;
    }
}

static void oled_show_status(void) {
    const ControlStatus *status = control_get_status();

    oled_set_cursor(0U, 0U);
    oled_write_string("PS2:");
    /*
     * 不在每次刷新时 oled_clear()。
     *
     * 原因：SSD1306 通过 I2C 写满 128x64 屏幕需要传很多字节。
     * 如果每 200ms 先清屏再重画，肉眼会看到一行一行被擦掉和重写，
     * 看起来像屏幕内容在递进/闪烁。
     *
     * 这里改为“固定位置覆盖显示”：每个字段都写固定宽度，
     * 旧字符会被后面的空格覆盖，屏幕就稳定多了。
     */
    oled_write_ps2_mode(status);
    if (status->ps2_connected != 0U) {
        oled_write_char(' ');
        oled_write_hex_byte(status->ps2_mode);
    } else {
        oled_write_string("   ");
    }
    oled_write_string(" EN:");
    oled_write_int(status->motor_enabled);
    oled_write_string("  ");

    oled_set_cursor(2U, 0U);
    oled_write_string("L:");
    oled_write_int_width(status->ps2_lx, 3U);
    oled_write_char('-');
    oled_write_int_width(status->ps2_ly, 3U);
    oled_write_string(" R:");
    oled_write_int_width(status->ps2_rx, 3U);
    oled_write_char('-');
    oled_write_int_width(status->ps2_ry, 3U);

    oled_set_cursor(4U, 0U);
    oled_write_string("TG:");
    oled_write_int_width(status->target[0], 3U);
    oled_write_char('-');
    oled_write_int_width(status->target[1], 3U);
    oled_write_char('-');
    oled_write_int_width(status->target[2], 3U);
    oled_write_char('-');
    oled_write_int_width(status->target[3], 3U);

    oled_set_cursor(6U, 0U);
    oled_write_string("M:");
    oled_write_int_width(status->measured[0], 3U);
    oled_write_char('-');
    oled_write_int_width(status->measured[1], 3U);
    oled_write_char('-');
    oled_write_int_width(status->measured[2], 3U);
    oled_write_char('-');
    oled_write_int_width(status->measured[3], 3U);
}

int main(void) {
    uint32_t last_control_tick;
    uint32_t last_led_tick;
    uint32_t last_oled_tick;

    /*
     * 初始化顺序：
     *
     * 1. gpio_clock_enable()
     *    打开 GPIOA/GPIOB/GPIOC 和 AFIO 的时钟。
     *    STM32 的外设默认不供时钟，不先打开时钟，后面配置引脚不会生效。
     *
     * 2. systick_init()
     *    配置 1ms 系统节拍。后面的 LED 闪烁、10ms 控制周期、OLED 刷新都靠 millis() 计时。
     *
     * 3. led_init()
     *    配置 PC13 板载 LED。只接一块最小系统板时，看到 LED 闪烁就是第一步成功。
     *
     * 4. oled_init()
     *    初始化 PB8/PB9 软件 I2C OLED。OLED 没接也不会影响主程序运行，只是没有显示。
     *
     * 5. pwm_init() 和 tb6612_init()
     *    配置 TIM1 四路 PWM 和 TB6612 方向脚。此时电机默认停止。
     *
     * 6. encoder_init() 和 ps2_init()
     *    配置编码器输入和 PS2 手柄通信引脚。
     *
     * 7. control_init()
     *    初始化四个轮子的 PID 状态。上电后必须按 START 才允许电机输出。
     */
    gpio_clock_enable();
    systick_init();
    led_init();
    oled_init();
    pwm_init();
    tb6612_init();
    encoder_init();
    ps2_init();
    control_init();

    last_control_tick = millis();
    last_led_tick = millis();
    last_oled_tick = millis();
    oled_show_status();

    while (1) {
        /*
         * PC13 LED 心跳。
         * 只要主循环还在正常跑，LED 就会按 LED_HEARTBEAT_MS 周期闪烁。
         * 如果 LED 不闪，优先检查下载是否成功、PC13 LED 是否低电平点亮、系统时钟是否正常。
         */
        if ((millis() - last_led_tick) >= LED_HEARTBEAT_MS) {
            last_led_tick += LED_HEARTBEAT_MS;
            led_toggle();
        }

        /* 固定 10ms 控制周期，不使用 delay_ms，避免循环抖动累积。 */
        if ((millis() - last_control_tick) >= CONTROL_PERIOD_MS) {
            last_control_tick += CONTROL_PERIOD_MS;
            control_update_10ms();
        }

        /*
         * OLED 刷新比控制周期慢很多。
         * OLED I2C 写屏比较耗时，没必要 10ms 刷一次；200ms 左右更适合人眼观察。
         */
        if ((millis() - last_oled_tick) >= OLED_REFRESH_MS) {
            last_oled_tick += OLED_REFRESH_MS;
            oled_show_status();
        }
    }
}
