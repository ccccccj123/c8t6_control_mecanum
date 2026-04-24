#ifndef DRIVERS_PS2_H
#define DRIVERS_PS2_H

#include <stdint.h>

#define PS2_BTN_SELECT    0x0001U
#define PS2_BTN_L3        0x0002U
#define PS2_BTN_R3        0x0004U
#define PS2_BTN_START     0x0008U
#define PS2_BTN_UP        0x0010U
#define PS2_BTN_RIGHT     0x0020U
#define PS2_BTN_DOWN      0x0040U
#define PS2_BTN_LEFT      0x0080U
#define PS2_BTN_L2        0x0100U
#define PS2_BTN_R2        0x0200U
#define PS2_BTN_L1        0x0400U
#define PS2_BTN_R1        0x0800U
#define PS2_BTN_TRIANGLE  0x1000U
#define PS2_BTN_CIRCLE    0x2000U
#define PS2_BTN_CROSS     0x4000U
#define PS2_BTN_SQUARE    0x8000U

typedef struct {
    uint8_t connected;
    uint16_t buttons;
    uint8_t rx;
    uint8_t ry;
    uint8_t lx;
    uint8_t ly;
} PS2State;

void ps2_init(void);
uint8_t ps2_read(PS2State *state);
uint8_t ps2_button_down(const PS2State *state, uint16_t button);

#endif
