#include <assert.h>
#include <stdio.h>

/*
 * 这个测试文件在电脑上运行，不依赖 STM32 硬件。
 * 作用是保护最核心的算法：麦轮混控符号、限幅比例、PID 限幅和积分复位。
 * 如果以后改了 App/mecanum.c 或 App/pid.c，先跑这个测试确认基础行为没坏。
 */

#include "App/mecanum.h"
#include "App/pid.h"
#include "Drivers/ps2.h"

static void test_forward_sets_all_wheels_positive(void) {
    /* 前进时四个轮子都应该同向。 */
    WheelSpeeds w = mecanum_mix(0, 500, 0, 1000);
    assert(w.front_left == 500);
    assert(w.front_right == 500);
    assert(w.rear_left == 500);
    assert(w.rear_right == 500);
}

static void test_strafe_right_uses_mecanum_sign_pattern(void) {
    /* 右平移时麦轮呈 + - - + 的典型符号模式。 */
    WheelSpeeds w = mecanum_mix(400, 0, 0, 1000);
    assert(w.front_left == 400);
    assert(w.front_right == -400);
    assert(w.rear_left == -400);
    assert(w.rear_right == 400);
}

static void test_clockwise_rotation_uses_opposite_sides(void) {
    /* 顺时针自转时左右两侧轮子方向相反。 */
    WheelSpeeds w = mecanum_mix(0, 0, 300, 1000);
    assert(w.front_left == 300);
    assert(w.front_right == -300);
    assert(w.rear_left == 300);
    assert(w.rear_right == -300);
}

static void test_mix_scales_to_limit_without_changing_ratio(void) {
    /* 多轴叠加超过限幅时，应等比例缩放，而不是单个轮子硬截断。 */
    WheelSpeeds w = mecanum_mix(800, 800, 800, 1000);
    assert(w.front_left == 1000);
    assert(w.front_right == -333);
    assert(w.rear_left == 333);
    assert(w.rear_right == 333);
}

static void test_pid_clamps_output(void) {
    /* 大误差时 PID 输出不能超过 PWM 量程。 */
    PIDController pid;
    pid_init(&pid, 200, 0, 0, 600, 1000);
    assert(pid_update(&pid, 10, 0) == 600);
}

static void test_pid_integral_accumulates_and_resets(void) {
    /* 积分项应能累积、限幅，并在停车/重新使能时清零。 */
    PIDController pid;
    pid_init(&pid, 0, 100, 0, 1000, 5);
    assert(pid_update(&pid, 3, 0) == 300);
    assert(pid_update(&pid, 3, 0) == 500);
    pid_reset(&pid);
    assert(pid_update(&pid, 3, 0) == 300);
}

static void test_digital_ps2_mode_forces_sticks_to_neutral(void) {
    /*
     * 数字模式(0x41)下没有可靠的模拟摇杆数据。
     * 为了避免按 START 后因为垃圾值导致电机自己转，代码应强制回中。
     */
    PS2State state = {
        .connected = 1U,
        .mode = PS2_MODE_DIGITAL,
        .buttons = 0U,
        .rx = 0U,
        .ry = 255U,
        .lx = 1U,
        .ly = 254U
    };

    ps2_sanitize_axes(&state);

    assert(ps2_has_analog_sticks(&state) == 0U);
    assert(state.rx == PS2_STICK_NEUTRAL);
    assert(state.ry == PS2_STICK_NEUTRAL);
    assert(state.lx == PS2_STICK_NEUTRAL);
    assert(state.ly == PS2_STICK_NEUTRAL);
}

static void test_ps2_mode_reconfigure_rule_matches_expected_modes(void) {
    PS2State disconnected = {0};
    PS2State digital = {.connected = 1U, .mode = PS2_MODE_DIGITAL};
    PS2State analog = {.connected = 1U, .mode = PS2_MODE_ANALOG_RED};
    PS2State pressure = {.connected = 1U, .mode = PS2_MODE_ANALOG_PRESSURE};

    assert(ps2_should_enable_analog_mode(&disconnected) == 1U);
    assert(ps2_should_enable_analog_mode(&digital) == 1U);
    assert(ps2_should_enable_analog_mode(&analog) == 0U);
    assert(ps2_should_enable_analog_mode(&pressure) == 0U);
}

int main(void) {
    test_forward_sets_all_wheels_positive();
    test_strafe_right_uses_mecanum_sign_pattern();
    test_clockwise_rotation_uses_opposite_sides();
    test_mix_scales_to_limit_without_changing_ratio();
    test_pid_clamps_output();
    test_pid_integral_accumulates_and_resets();
    test_digital_ps2_mode_forces_sticks_to_neutral();
    test_ps2_mode_reconfigure_rule_matches_expected_modes();
    puts("All host tests passed");
    return 0;
}
