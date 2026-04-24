#include "control.h"
#include "config.h"
#include "mecanum.h"
#include "pid.h"
#include "encoder.h"
#include "ps2.h"
#include "tb6612.h"

static PIDController wheel_pid[MOTOR_COUNT];
static uint8_t enabled;
static int16_t speed_limit = SPEED_LIMIT_TICKS;

static int16_t axis_to_command(uint8_t raw, int16_t limit, uint8_t invert) {
    int16_t centered = (int16_t)raw - PS2_NEUTRAL;
    if (invert != 0U) {
        centered = (int16_t)-centered;
    }
    if (centered > -JOYSTICK_DEADZONE && centered < JOYSTICK_DEADZONE) {
        return 0;
    }
    return (int16_t)((int32_t)centered * limit / PS2_AXIS_MAX);
}

void control_init(void) {
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        pid_init(&wheel_pid[i], PID_KP, PID_KI, PID_KD,
                 PID_OUTPUT_LIMIT, PID_INTEGRAL_LIMIT);
    }
    enabled = 0U;
}

void control_update_10ms(void) {
    PS2State ps2;
    WheelSpeeds target;
    int16_t measured[MOTOR_COUNT];
    int16_t output[MOTOR_COUNT];

    if (ps2_read(&ps2) == 0U) {
        tb6612_stop_all();
        enabled = 0U;
        return;
    }

    if (ps2_button_down(&ps2, PS2_BTN_START) != 0U) {
        enabled = 1U;
        tb6612_set_standby(1U);
    }
    if (ps2_button_down(&ps2, PS2_BTN_SELECT) != 0U) {
        enabled = 0U;
        tb6612_stop_all();
    }
    speed_limit = ps2_button_down(&ps2, PS2_BTN_L1) ? SLOW_SPEED_TICKS : SPEED_LIMIT_TICKS;

    target = mecanum_mix(
        axis_to_command(ps2.lx, speed_limit, 0U),
        axis_to_command(ps2.ly, speed_limit, 1U),
        axis_to_command(ps2.rx, speed_limit, 0U),
        speed_limit);

    measured[MOTOR_FRONT_LEFT] = encoder_read_delta(MOTOR_FRONT_LEFT);
    measured[MOTOR_FRONT_RIGHT] = encoder_read_delta(MOTOR_FRONT_RIGHT);
    measured[MOTOR_REAR_LEFT] = encoder_read_delta(MOTOR_REAR_LEFT);
    measured[MOTOR_REAR_RIGHT] = encoder_read_delta(MOTOR_REAR_RIGHT);

    if (enabled == 0U) {
        tb6612_stop_all();
        return;
    }

    output[MOTOR_FRONT_LEFT] = pid_update(&wheel_pid[MOTOR_FRONT_LEFT],
                                          target.front_left,
                                          measured[MOTOR_FRONT_LEFT]);
    output[MOTOR_FRONT_RIGHT] = pid_update(&wheel_pid[MOTOR_FRONT_RIGHT],
                                           target.front_right,
                                           measured[MOTOR_FRONT_RIGHT]);
    output[MOTOR_REAR_LEFT] = pid_update(&wheel_pid[MOTOR_REAR_LEFT],
                                         target.rear_left,
                                         measured[MOTOR_REAR_LEFT]);
    output[MOTOR_REAR_RIGHT] = pid_update(&wheel_pid[MOTOR_REAR_RIGHT],
                                          target.rear_right,
                                          measured[MOTOR_REAR_RIGHT]);

    tb6612_set_motor(MOTOR_FRONT_LEFT, output[MOTOR_FRONT_LEFT]);
    tb6612_set_motor(MOTOR_FRONT_RIGHT, output[MOTOR_FRONT_RIGHT]);
    tb6612_set_motor(MOTOR_REAR_LEFT, output[MOTOR_REAR_LEFT]);
    tb6612_set_motor(MOTOR_REAR_RIGHT, output[MOTOR_REAR_RIGHT]);
}
