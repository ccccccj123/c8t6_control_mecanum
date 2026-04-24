#ifndef DRIVERS_TB6612_H
#define DRIVERS_TB6612_H

#include <stdint.h>

typedef enum {
    /* 电机顺序必须和运动学、编码器读取顺序保持一致。 */
    MOTOR_FRONT_LEFT = 0,
    MOTOR_FRONT_RIGHT,
    MOTOR_REAR_LEFT,
    MOTOR_REAR_RIGHT,
    MOTOR_COUNT
} MotorId;

/* 配置 TB6612 方向引脚，并默认拉高 STBY。 */
void tb6612_init(void);

/* 控制两块 TB6612 共用的 STBY 引脚，0 为待机，1 为允许输出。 */
void tb6612_set_standby(uint8_t enabled);

/* command 为带符号 PWM：正反转由 IN1/IN2 决定，幅值由 PWM 占空比决定。 */
void tb6612_set_motor(MotorId motor, int16_t command);

/* 四个通道全部滑行停止，适合掉线/急停时调用。 */
void tb6612_stop_all(void);

#endif
