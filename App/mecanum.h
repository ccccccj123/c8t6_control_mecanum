#ifndef APP_MECANUM_H
#define APP_MECANUM_H

#include <stdint.h>

typedef struct {
    /* 四个轮子的目标速度，单位为每个控制周期的编码器计数增量。 */
    int16_t front_left;
    int16_t front_right;
    int16_t rear_left;
    int16_t rear_right;
} WheelSpeeds;

/*
 * 麦轮运动学混控。
 * vx: 左右平移，正数表示向右。
 * vy: 前后平移，正数表示向前。
 * wz: 自转，正数表示顺时针。
 * limit: 单轮输出限幅，超过后会等比例缩放，保持方向比例不变。
 */
WheelSpeeds mecanum_mix(int16_t vx, int16_t vy, int16_t wz, int16_t limit);

#endif
