#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#define SYSCLK_HZ                 72000000UL
#define CONTROL_PERIOD_MS         10U
#define PWM_FREQUENCY_HZ          20000U
#define PWM_MAX_DUTY              1000

#define SPEED_LIMIT_TICKS         900
#define SLOW_SPEED_TICKS          450
#define JOYSTICK_DEADZONE         18

#define PID_KP                    18
#define PID_KI                    2
#define PID_KD                    0
#define PID_OUTPUT_LIMIT          PWM_MAX_DUTY
#define PID_INTEGRAL_LIMIT        5000

#define PS2_NEUTRAL               128
#define PS2_AXIS_MAX              127

#endif
