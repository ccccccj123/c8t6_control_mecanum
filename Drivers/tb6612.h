#ifndef DRIVERS_TB6612_H
#define DRIVERS_TB6612_H

#include <stdint.h>

typedef enum {
    MOTOR_FRONT_LEFT = 0,
    MOTOR_FRONT_RIGHT,
    MOTOR_REAR_LEFT,
    MOTOR_REAR_RIGHT,
    MOTOR_COUNT
} MotorId;

void tb6612_init(void);
void tb6612_set_standby(uint8_t enabled);
void tb6612_set_motor(MotorId motor, int16_t command);
void tb6612_stop_all(void);

#endif
