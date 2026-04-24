#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <stdint.h>

/* 初始化四个轮子的 PID 状态，默认上电后电机不使能。 */
void control_init(void);

/* 10ms 调用一次：读 PS2、算目标速度、读编码器、PID、输出电机。 */
void control_update_10ms(void);

#endif
