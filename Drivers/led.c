#include "led.h"
#include "gpio.h"

#define LED_PORT GPIOC
#define LED_PIN  13

static uint8_t led_on;

void led_init(void) {
    /* PC13 输出能力较弱，但驱动板载 LED 足够。 */
    gpio_config(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
    led_set(0U);
}

void led_set(uint8_t on) {
    led_on = (on != 0U);
    /* 板载 LED 低电平点亮，所以这里取反输出。 */
    gpio_write(LED_PORT, LED_PIN, led_on ? 0U : 1U);
}

void led_toggle(void) {
    led_set((uint8_t)!led_on);
}
