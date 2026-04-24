#include "mecanum.h"

static int16_t abs16(int16_t value) {
    return value < 0 ? (int16_t)-value : value;
}

static int16_t clamp16(int32_t value, int16_t limit) {
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return (int16_t)-limit;
    }
    return (int16_t)value;
}

WheelSpeeds mecanum_mix(int16_t vx, int16_t vy, int16_t wz, int16_t limit) {
    int32_t fl = (int32_t)vy + vx + wz;
    int32_t fr = (int32_t)vy - vx - wz;
    int32_t rl = (int32_t)vy - vx + wz;
    int32_t rr = (int32_t)vy + vx - wz;

    int32_t max_abs = abs16((int16_t)fl);
    int32_t candidate = abs16((int16_t)fr);
    if (candidate > max_abs) {
        max_abs = candidate;
    }
    candidate = abs16((int16_t)rl);
    if (candidate > max_abs) {
        max_abs = candidate;
    }
    candidate = abs16((int16_t)rr);
    if (candidate > max_abs) {
        max_abs = candidate;
    }

    if (limit > 0 && max_abs > limit) {
        fl = fl * limit / max_abs;
        fr = fr * limit / max_abs;
        rl = rl * limit / max_abs;
        rr = rr * limit / max_abs;
    }

    WheelSpeeds speeds;
    speeds.front_left = clamp16(fl, limit);
    speeds.front_right = clamp16(fr, limit);
    speeds.rear_left = clamp16(rl, limit);
    speeds.rear_right = clamp16(rr, limit);
    return speeds;
}
