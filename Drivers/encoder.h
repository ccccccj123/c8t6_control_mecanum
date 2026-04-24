#ifndef DRIVERS_ENCODER_H
#define DRIVERS_ENCODER_H

#include <stdint.h>
#include "tb6612.h"

/* 初始化三路硬件编码器和一路 EXTI 软件编码器。 */
void encoder_init(void);

/*
 * 清除编码器速度计算基准。
 * 上电、停车、手柄掉线后调用一次，避免下一次读取时把旧累计值当成 10ms 速度。
 */
void encoder_reset(void);

/* 返回上一次读取到现在的增量，控制周期内调用一次即为当前轮速。 */
int16_t encoder_read_delta(MotorId motor);

#endif
