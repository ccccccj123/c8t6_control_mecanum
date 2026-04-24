#ifndef DRIVERS_OLED_H
#define DRIVERS_OLED_H

#include <stdint.h>

/*
 * 0.96 寸 SSD1306 I2C OLED 调试屏驱动。
 * 默认接线：SCL=PB8，SDA=PB9，VCC=3V3，GND=GND。
 */
void oled_init(void);
void oled_clear(void);
void oled_set_cursor(uint8_t page, uint8_t column);
void oled_write_char(char ch);
void oled_write_string(const char *text);
void oled_write_int(int16_t value);

#endif
