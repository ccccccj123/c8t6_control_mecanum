#include "pid.h"

/* PID 输出限幅，避免返回值超过 TB6612 PWM 量程。 */
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
    /*
     * target 和 measured 都是“每 10ms 编码器计数增量”。
     * 这样可以少做浮点和 RPM 换算，方便在 C8T6 上保持简单可靠。
     */
    int16_t error = (int16_t)(target - measured);
    int16_t derivative = (int16_t)(error - pid->previous_error);

    /* 积分限幅用于防止长时间卡住或目标过高时出现积分饱和。 */
    pid->integral += error;
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }

    pid->previous_error = error;

    /* 整数 PID：参数越大反应越猛。初调时建议先调 KP，再少量加 KI。 */
    int32_t output = (int32_t)pid->kp * error;
    output += (int32_t)pid->ki * pid->integral;
    output += (int32_t)pid->kd * derivative;
    return clamp_i16(output, pid->output_limit);
}
