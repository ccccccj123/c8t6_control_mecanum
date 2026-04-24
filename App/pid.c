#include "pid.h"

static int16_t clamp_i16(int32_t value, int16_t limit) {
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return (int16_t)-limit;
    }
    return (int16_t)value;
}

void pid_init(PIDController *pid, int16_t kp, int16_t ki, int16_t kd,
              int16_t output_limit, int16_t integral_limit) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->output_limit = output_limit;
    pid->integral_limit = integral_limit;
    pid_reset(pid);
}

void pid_reset(PIDController *pid) {
    pid->integral = 0;
    pid->previous_error = 0;
}

int16_t pid_update(PIDController *pid, int16_t target, int16_t measured) {
    int16_t error = (int16_t)(target - measured);
    int16_t derivative = (int16_t)(error - pid->previous_error);

    pid->integral += error;
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }

    pid->previous_error = error;

    int32_t output = (int32_t)pid->kp * error;
    output += (int32_t)pid->ki * pid->integral;
    output += (int32_t)pid->kd * derivative;
    return clamp_i16(output, pid->output_limit);
}
