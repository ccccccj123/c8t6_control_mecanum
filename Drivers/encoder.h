#ifndef DRIVERS_ENCODER_H
#define DRIVERS_ENCODER_H

#include <stdint.h>
#include "tb6612.h"

void encoder_init(void);
int16_t encoder_read_delta(MotorId motor);

#endif
