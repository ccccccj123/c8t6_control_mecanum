#include "oled.h"
#include "gpio.h"
#include "systick.h"

#define OLED_SCL_PORT GPIOB
#define OLED_SCL_PIN  8
#define OLED_SDA_PORT GPIOB
#define OLED_SDA_PIN  9
#define OLED_ADDR     0x78U

static void i2c_delay(void) {
    for (volatile uint16_t i = 0; i < 30U; i++) {
    }
}

static void scl(uint8_t high) {
    gpio_write(OLED_SCL_PORT, OLED_SCL_PIN, high);
}

static void sda(uint8_t high) {
    gpio_write(OLED_SDA_PORT, OLED_SDA_PIN, high);
}

static void i2c_start(void) {
    sda(1U);
    scl(1U);
    i2c_delay();
    sda(0U);
    i2c_delay();
    scl(0U);
}

static void i2c_stop(void) {
    sda(0U);
    scl(1U);
    i2c_delay();
    sda(1U);
    i2c_delay();
}

static void i2c_write_byte(uint8_t data) {
    for (uint8_t i = 0U; i < 8U; i++) {
        sda((uint8_t)((data & 0x80U) != 0U));
        data <<= 1;
        scl(1U);
        i2c_delay();
        scl(0U);
        i2c_delay();
    }

    /*
     * 第 9 个时钟是 ACK 位。为了让驱动简单可靠，这里不读取 ACK。
     * 如果 OLED 没接好，程序不会卡死，只是屏幕不显示。
     */
    sda(1U);
    scl(1U);
    i2c_delay();
    scl(0U);
}

static void oled_write(uint8_t control, uint8_t data) {
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(control);
    i2c_write_byte(data);
    i2c_stop();
}

static void oled_cmd(uint8_t cmd) {
    oled_write(0x00U, cmd);
}

static void oled_data(uint8_t data) {
    oled_write(0x40U, data);
}

static const uint8_t glyph_space[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t glyph_dash[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
static const uint8_t glyph_colon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
static const uint8_t glyph_question[5] = {0x02, 0x01, 0x51, 0x09, 0x06};
static const uint8_t glyph_digits[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46},
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36},
    {0x06, 0x49, 0x49, 0x29, 0x1E},
};
static const uint8_t glyph_letters[26][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43},
};

static const uint8_t *glyph_for(char ch) {
    if (ch >= '0' && ch <= '9') {
        return glyph_digits[ch - '0'];
    }
    if (ch >= 'a' && ch <= 'z') {
        ch = (char)(ch - 'a' + 'A');
    }
    if (ch >= 'A' && ch <= 'Z') {
        return glyph_letters[ch - 'A'];
    }
    if (ch == '-') {
        return glyph_dash;
    }
    if (ch == ':') {
        return glyph_colon;
    }
    if (ch == ' ') {
        return glyph_space;
    }
    return glyph_question;
}

void oled_init(void) {
    gpio_config(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_OD);
    gpio_config(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUT_OD);
    scl(1U);
    sda(1U);
    delay_ms(50U);

    oled_cmd(0xAEU);
    oled_cmd(0x20U);
    oled_cmd(0x02U);
    oled_cmd(0xB0U);
    oled_cmd(0xC8U);
    oled_cmd(0x00U);
    oled_cmd(0x10U);
    oled_cmd(0x40U);
    oled_cmd(0x81U);
    oled_cmd(0x7FU);
    oled_cmd(0xA1U);
    oled_cmd(0xA6U);
    oled_cmd(0xA8U);
    oled_cmd(0x3FU);
    oled_cmd(0xA4U);
    oled_cmd(0xD3U);
    oled_cmd(0x00U);
    oled_cmd(0xD5U);
    oled_cmd(0x80U);
    oled_cmd(0xD9U);
    oled_cmd(0xF1U);
    oled_cmd(0xDAU);
    oled_cmd(0x12U);
    oled_cmd(0xDBU);
    oled_cmd(0x40U);
    oled_cmd(0x8DU);
    oled_cmd(0x14U);
    oled_cmd(0xAFU);
    oled_clear();
}

void oled_clear(void) {
    for (uint8_t page = 0U; page < 8U; page++) {
        oled_set_cursor(page, 0U);
        for (uint8_t col = 0U; col < 128U; col++) {
            oled_data(0x00U);
        }
    }
    oled_set_cursor(0U, 0U);
}

void oled_set_cursor(uint8_t page, uint8_t column) {
    oled_cmd((uint8_t)(0xB0U | (page & 0x07U)));
    oled_cmd((uint8_t)(0x00U | (column & 0x0FU)));
    oled_cmd((uint8_t)(0x10U | ((column >> 4) & 0x0FU)));
}

void oled_write_char(char ch) {
    const uint8_t *glyph = glyph_for(ch);
    for (uint8_t i = 0U; i < 5U; i++) {
        oled_data(glyph[i]);
    }
    oled_data(0x00U);
}

void oled_write_string(const char *text) {
    while (*text != '\0') {
        oled_write_char(*text++);
    }
}

void oled_write_int(int16_t value) {
    char buf[7];
    uint8_t pos = 0U;
    uint16_t number;
    char rev[6];
    uint8_t rev_pos = 0U;

    if (value < 0) {
        buf[pos++] = '-';
        number = (uint16_t)(-value);
    } else {
        number = (uint16_t)value;
    }

    do {
        rev[rev_pos++] = (char)('0' + (number % 10U));
        number /= 10U;
    } while (number != 0U && rev_pos < sizeof(rev));

    while (rev_pos > 0U) {
        buf[pos++] = rev[--rev_pos];
    }
    buf[pos] = '\0';
    oled_write_string(buf);
}
