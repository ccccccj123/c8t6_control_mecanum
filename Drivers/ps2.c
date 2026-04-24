#include "ps2.h"
#include "gpio.h"

#define PS2_ATT_PORT GPIOB
#define PS2_ATT_PIN  12
#define PS2_CLK_PORT GPIOB
#define PS2_CLK_PIN  13
#define PS2_DAT_PORT GPIOB
#define PS2_DAT_PIN  14
#define PS2_CMD_PORT GPIOB
#define PS2_CMD_PIN  15

static void ps2_delay(void) {
    for (volatile uint16_t i = 0; i < 80U; i++) {
    }
}

static uint8_t ps2_transfer(uint8_t out) {
    uint8_t in = 0U;
    for (uint8_t i = 0; i < 8U; i++) {
        gpio_write(PS2_CMD_PORT, PS2_CMD_PIN, (uint8_t)(out & 0x1U));
        gpio_write(PS2_CLK_PORT, PS2_CLK_PIN, 0U);
        ps2_delay();
        if (gpio_read(PS2_DAT_PORT, PS2_DAT_PIN) != 0U) {
            in |= (uint8_t)(1U << i);
        }
        gpio_write(PS2_CLK_PORT, PS2_CLK_PIN, 1U);
        ps2_delay();
        out >>= 1;
    }
    return in;
}

void ps2_init(void) {
    gpio_config(PS2_ATT_PORT, PS2_ATT_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
    gpio_config(PS2_CLK_PORT, PS2_CLK_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
    gpio_config(PS2_CMD_PORT, PS2_CMD_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_PP);
    gpio_config(PS2_DAT_PORT, PS2_DAT_PIN, GPIO_MODE_INPUT, GPIO_CNF_IN_PULL);
    PS2_DAT_PORT->ODR |= 1UL << PS2_DAT_PIN;
    gpio_write(PS2_ATT_PORT, PS2_ATT_PIN, 1U);
    gpio_write(PS2_CLK_PORT, PS2_CLK_PIN, 1U);
    gpio_write(PS2_CMD_PORT, PS2_CMD_PIN, 1U);
}

uint8_t ps2_read(PS2State *state) {
    uint8_t data[9];

    gpio_write(PS2_ATT_PORT, PS2_ATT_PIN, 0U);
    ps2_delay();
    data[0] = ps2_transfer(0x01U);
    data[1] = ps2_transfer(0x42U);
    data[2] = ps2_transfer(0x00U);
    for (uint8_t i = 3U; i < 9U; i++) {
        data[i] = ps2_transfer(0x00U);
    }
    gpio_write(PS2_ATT_PORT, PS2_ATT_PIN, 1U);

    state->connected = (data[1] == 0x41U || data[1] == 0x73U || data[1] == 0x79U);
    state->buttons = (uint16_t)(~((uint16_t)data[4] << 8 | data[3]));
    state->rx = data[5];
    state->ry = data[6];
    state->lx = data[7];
    state->ly = data[8];
    return state->connected;
}

uint8_t ps2_button_down(const PS2State *state, uint16_t button) {
    return (uint8_t)((state->buttons & button) != 0U);
}
