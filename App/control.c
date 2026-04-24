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

/*
 * 将 PS2 摇杆 0-255 原始值转换为带符号速度命令。
 * invert 用于处理前后轴：PS2 向上推通常数值变小，而小车前进希望是正数。
 */
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

    /* 手柄掉线时立即停车，并要求重新按 START 才能恢复输出。 */
    if (ps2_read(&ps2) == 0U) {
        tb6612_stop_all();
        enabled = 0U;
        return;
    }

    /* START 使能，SELECT 停车；这是调车时的基础安全开关。 */
    if (ps2_button_down(&ps2, PS2_BTN_START) != 0U) {
        enabled = 1U;
        tb6612_set_standby(1U);
    }
    if (ps2_button_down(&ps2, PS2_BTN_SELECT) != 0U) {
        enabled = 0U;
        tb6612_stop_all();
    }
    /* 按住 L1 进入慢速模式，适合第一次上电测试方向和 PID。 */
    speed_limit = ps2_button_down(&ps2, PS2_BTN_L1) ? SLOW_SPEED_TICKS : SPEED_LIMIT_TICKS;

    /*
     * 左摇杆控制平移，右摇杆 X 轴控制自转。
     *
     * 注意：没有陀螺仪时，小车不知道自己的实际朝向。
     * 所以这里做的是“车体坐标系”控制：
     * - 手柄向前：车头方向前进；
     * - 手柄向右：相对车身向右平移；
     * - 右摇杆：让车旋转。
     * 如果地面打滑或左右电机差异较大，小车角度会慢慢偏，这是无陀螺仪方案的正常限制。
     */
    target = mecanum_mix(
        axis_to_command(ps2.lx, speed_limit, 0U),
        axis_to_command(ps2.ly, speed_limit, 1U),
        axis_to_command(ps2.rx, speed_limit, 0U),
        speed_limit);

    /* 读取每个控制周期内的编码器增量，作为实际轮速反馈。 */
    measured[MOTOR_FRONT_LEFT] = encoder_read_delta(MOTOR_FRONT_LEFT);
    measured[MOTOR_FRONT_RIGHT] = encoder_read_delta(MOTOR_FRONT_RIGHT);
    measured[MOTOR_REAR_LEFT] = encoder_read_delta(MOTOR_REAR_LEFT);
    measured[MOTOR_REAR_RIGHT] = encoder_read_delta(MOTOR_REAR_RIGHT);

    /*
     * 即使未使能，也仍然读取了一次编码器增量。
     * 这样按 START 重新使能时，不会把停车期间手动拨轮子的累计计数一次性送进 PID。
     */
    if (enabled == 0U) {
        tb6612_stop_all();
        return;
    }

    /*
     * 每个轮子独立闭环。
     * target 是麦轮运动学算出的目标计数增量，measured 是编码器实测增量。
     * PID 输出是带符号 PWM，正负方向交给 TB6612，绝对值交给 TIM1 PWM。
     */
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
