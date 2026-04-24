#include <assert.h>
#include <stdio.h>

#include "App/mecanum.h"
#include "App/pid.h"

static void test_forward_sets_all_wheels_positive(void) {
    WheelSpeeds w = mecanum_mix(0, 500, 0, 1000);
    assert(w.front_left == 500);
    assert(w.front_right == 500);
    assert(w.rear_left == 500);
    assert(w.rear_right == 500);
}

static void test_strafe_right_uses_mecanum_sign_pattern(void) {
    WheelSpeeds w = mecanum_mix(400, 0, 0, 1000);
    assert(w.front_left == 400);
    assert(w.front_right == -400);
    assert(w.rear_left == -400);
    assert(w.rear_right == 400);
}

static void test_clockwise_rotation_uses_opposite_sides(void) {
    WheelSpeeds w = mecanum_mix(0, 0, 300, 1000);
    assert(w.front_left == 300);
    assert(w.front_right == -300);
    assert(w.rear_left == 300);
    assert(w.rear_right == -300);
}

static void test_mix_scales_to_limit_without_changing_ratio(void) {
    WheelSpeeds w = mecanum_mix(800, 800, 800, 1000);
    assert(w.front_left == 1000);
    assert(w.front_right == -333);
    assert(w.rear_left == 333);
    assert(w.rear_right == 333);
}

static void test_pid_clamps_output(void) {
    PIDController pid;
    pid_init(&pid, 200, 0, 0, 600, 1000);
    assert(pid_update(&pid, 10, 0) == 600);
}

static void test_pid_integral_accumulates_and_resets(void) {
    PIDController pid;
    pid_init(&pid, 0, 100, 0, 1000, 5);
    assert(pid_update(&pid, 3, 0) == 300);
    assert(pid_update(&pid, 3, 0) == 500);
    pid_reset(&pid);
    assert(pid_update(&pid, 3, 0) == 300);
}

int main(void) {
    test_forward_sets_all_wheels_positive();
    test_strafe_right_uses_mecanum_sign_pattern();
    test_clockwise_rotation_uses_opposite_sides();
    test_mix_scales_to_limit_without_changing_ratio();
    test_pid_clamps_output();
    test_pid_integral_accumulates_and_resets();
    puts("All host tests passed");
    return 0;
}
