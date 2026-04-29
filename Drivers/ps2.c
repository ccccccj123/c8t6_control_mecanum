#include "ps2.h"
#include "gpio.h"
#include "systick.h"

#define PS2_ATT_PORT GPIOB
#define PS2_ATT_PIN  12
#define PS2_CLK_PORT GPIOB
#define PS2_CLK_PIN  13
#define PS2_DAT_PORT GPIOB
#define PS2_DAT_PIN  14
#define PS2_CMD_PORT GPIOB
#define PS2_CMD_PIN  15

/* 简单短延时，用于 PS2 位时序。Keil 优化等级改变后如通信不稳，可适当加大循环值。 */
static void ps2_delay(void) {
    for (volatile uint16_t i = 0; i < 80U; i++) {
    }
}

static void ps2_frame_delay(void) {
    delay_ms(1U);
}

static uint8_t ps2_transfer(uint8_t out) {
    uint8_t in = 0U;
    for (uint8_t i = 0; i < 8U; i++) {
        /* PS2 协议低位先传，CMD 在 CLK 拉低前准备好。 */
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

static void ps2_exchange(const uint8_t *tx, uint8_t *rx, uint8_t length) {
    if (tx == 0 || length == 0U) {
        return;
    }

    gpio_write(PS2_ATT_PORT, PS2_ATT_PIN, 0U);
    ps2_delay();
    for (uint8_t i = 0U; i < length; i++) {
        uint8_t in = ps2_transfer(tx[i]);
        if (rx != 0) {
            rx[i] = in;
        }
    }
    gpio_write(PS2_ATT_PORT, PS2_ATT_PIN, 1U);
    ps2_frame_delay();
}

static void ps2_send_config(const uint8_t *tx, uint8_t length) {
    ps2_exchange(tx, 0, length);
}

static void ps2_try_enable_analog_mode(void) {
    static const uint8_t short_poll[] = {0x01U, 0x42U, 0x00U, 0x00U, 0x00U};
    static const uint8_t enter_config[] = {0x01U, 0x43U, 0x00U, 0x01U, 0x00U,
                                           0x00U, 0x00U, 0x00U, 0x00U};
    static const uint8_t enable_analog[] = {0x01U, 0x44U, 0x00U, 0x01U, 0x03U,
                                            0x00U, 0x00U, 0x00U, 0x00U};
    static const uint8_t exit_config[] = {0x01U, 0x43U, 0x00U, 0x00U, 0x5AU,
                                          0x5AU, 0x5AU, 0x5AU, 0x5AU};
    PS2State state;

    for (uint8_t attempt = 0U; attempt < 3U; attempt++) {
        if (ps2_read(&state) != 0U && ps2_should_enable_analog_mode(&state) == 0U) {
            return;
        }

        ps2_send_config(short_poll, sizeof(short_poll));
        ps2_send_config(short_poll, sizeof(short_poll));
        ps2_send_config(short_poll, sizeof(short_poll));
        ps2_send_config(enter_config, sizeof(enter_config));
        ps2_send_config(enable_analog, sizeof(enable_analog));
        ps2_send_config(exit_config, sizeof(exit_config));
        delay_ms(10U);
    }
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
    delay_ms(20U);
    ps2_try_enable_analog_mode();
}

uint8_t ps2_read(PS2State *state) {
    static const uint8_t poll[] = {0x01U, 0x42U, 0x00U, 0x00U, 0x00U,
                                   0x00U, 0x00U, 0x00U, 0x00U};
    uint8_t data[9];

    /*
     * 标准 PS2 轮询命令：
     * 0x01 起始，0x42 请求数据，后续 0x00 用于时钟换回手柄数据。
     */
    ps2_exchange(poll, data, sizeof(data));

    /* 记录当前模式字节，供上层区分数字模式和模拟模式。 */
    state->mode = data[1];
    /* 0x41/0x73/0x79 是常见数字/模拟/压力模式 ID。 */
    state->connected = (state->mode == PS2_MODE_DIGITAL ||
                        state->mode == PS2_MODE_ANALOG_RED ||
                        state->mode == PS2_MODE_ANALOG_PRESSURE);

    /* PS2 按键原始数据是低有效，这里取反后统一成 1=按下。 */
    state->buttons = (uint16_t)(~((uint16_t)data[4] << 8 | data[3]));
    state->rx = data[5];
    state->ry = data[6];
    state->lx = data[7];
    state->ly = data[8];
    /*
     * 数字模式没有可靠摇杆数据。
     * 为避免 START 使能后用到非中位垃圾值，这里统一把摇杆钳回中位。
     */
    ps2_sanitize_axes(state);
    return state->connected;
}

uint8_t ps2_button_down(const PS2State *state, uint16_t button) {
    return (uint8_t)((state->buttons & button) != 0U);
}
