#ifndef APP_PID_H
#define APP_PID_H

#include <stdint.h>

typedef struct {
    int16_t kp;
    int16_t ki;
    int16_t kd;
    int32_t integral;
    int16_t previous_error;
    int16_t output_limit;
    int16_t integral_limit;
} PIDController;

void pid_init(PIDController *pid, int16_t kp, int16_t ki, int16_t kd,
              int16_t output_limit, int16_t integral_limit);
void pid_reset(PIDController *pid);
int16_t pid_update(PIDController *pid, int16_t target, int16_t measured);

#endif
