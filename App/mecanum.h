#ifndef APP_MECANUM_H
#define APP_MECANUM_H

#include <stdint.h>

typedef struct {
    int16_t front_left;
    int16_t front_right;
    int16_t rear_left;
    int16_t rear_right;
} WheelSpeeds;

WheelSpeeds mecanum_mix(int16_t vx, int16_t vy, int16_t wz, int16_t limit);

#endif
