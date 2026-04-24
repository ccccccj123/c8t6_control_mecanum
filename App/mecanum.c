#include "mecanum.h"

/* 小范围 int16_t 绝对值工具，用于寻找四轮混控后的最大幅值。 */
static int16_t abs16(int16_t value) {
    return value < 0 ? (int16_t)-value : value;
}

/* 对最终单轮目标做保护限幅，防止异常输入越过控制器量程。 */
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
    /*
     * X 型麦轮常用混控公式：
     * FL = vy + vx + wz
     * FR = vy - vx - wz
     * RL = vy - vx + wz
     * RR = vy + vx - wz
     *
     * 如果实车某个轮子方向相反，优先对调对应电机线或编码器 A/B；
     * 其次再改 tb6612_set_motor() 的方向，不建议先改这里的运动学公式。
     */
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

    /*
     * 当平移和旋转叠加时，某些轮子的目标可能超过上限。
     * 这里做等比例缩放，而不是单独截断，避免运动方向被破坏。
     */
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
