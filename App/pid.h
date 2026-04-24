#ifndef APP_PID_H
#define APP_PID_H

#include <stdint.h>

typedef struct {
    /* 比例、积分、微分增益。当前实现采用整数直接计算，不做定点小数缩放。 */
    int16_t kp;
    int16_t ki;
    int16_t kd;

    /* 积分项和上一周期误差，用于速度闭环连续计算。 */
    int32_t integral;
    int16_t previous_error;

    /* 输出限幅对应 PWM_MAX_DUTY，积分限幅用于抑制长时间堵转时积分过大。 */
    int16_t output_limit;
    int16_t integral_limit;
} PIDController;

/* 初始化单个 PID 控制器，四个轮子可以各自使用一份 PIDController。 */
void pid_init(PIDController *pid, int16_t kp, int16_t ki, int16_t kd,
              int16_t output_limit, int16_t integral_limit);

/* 停车或重新使能前调用，清掉历史积分和微分记忆。 */
void pid_reset(PIDController *pid);

/* 输入目标/实测速度，返回带符号 PWM 命令；正负号交给 TB6612 控制方向。 */
int16_t pid_update(PIDController *pid, int16_t target, int16_t measured);

#endif
