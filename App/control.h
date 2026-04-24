#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <stdint.h>

typedef struct {
    uint8_t ps2_connected;
    uint8_t motor_enabled;
    int16_t target[4];
    int16_t measured[4];
    int16_t output[4];
} ControlStatus;

/* 初始化四个轮子的 PID 状态，默认上电后电机不使能。 */
void control_init(void);

/* 10ms 调用一次：读 PS2、算目标速度、读编码器、PID、输出电机。 */
void control_update_10ms(void);

/* 给 OLED/串口调试读取最近一次控制状态。 */
const ControlStatus *control_get_status(void);

#endif
