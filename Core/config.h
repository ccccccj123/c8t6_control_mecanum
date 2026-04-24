#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

/*
 * 全局参数集中放在这里，后续调车主要改这个文件。
 * 单位说明：
 * - *_TICKS 表示 10ms 控制周期内编码器计数增量，不是 RPM。
 * - PID 参数是整数增益，pid_update() 内部不做 /100 缩放。
 */

/* STM32F103C8T6 使用外部 8MHz 晶振，经 PLL 倍频到 72MHz。 */
#define SYSCLK_HZ                 72000000UL

/* 速度闭环周期。10ms 对霍尔 370 电机比较稳，太短会让低速计数抖动明显。 */
#define CONTROL_PERIOD_MS         10U

/* TB6612 PWM 频率。20kHz 基本避开人耳可闻范围。 */
#define PWM_FREQUENCY_HZ          20000U

/* PWM 占空比软件量程，1000 表示 100%。 */
#define PWM_MAX_DUTY              1000

/*
 * 普通/慢速模式下的单轮目标速度上限，按编码器每 10ms 的计数增量计算。
 *
 * 调车建议：
 * 1. 第一次上地前先把 SPEED_LIMIT_TICKS 调小，例如 250-400；
 * 2. 确认四轮方向、编码器方向都正确后再慢慢加大；
 * 3. 如果车一给油就抖，先降低 SPEED_LIMIT_TICKS 和 PID_KP。
 */
#define SPEED_LIMIT_TICKS         900
#define SLOW_SPEED_TICKS          450

/* PS2 摇杆中位附近死区，避免手柄轻微漂移导致小车自己慢慢动。 */
#define JOYSTICK_DEADZONE         18

/*
 * 四个轮子共用一组初始 PID。先让车能稳，再按实际电机逐个微调。
 *
 * 粗调顺序：
 * - 先只用 KP：KI=0, KD=0，慢慢加 KP，直到速度跟随明显但不持续振荡；
 * - 再加少量 KI：用于消除低速爬行时的静差；
 * - KD 对带减速箱小车通常不是必需，编码器计数噪声会让 KD 放大抖动。
 */
#define PID_KP                    18
#define PID_KI                    2
#define PID_KD                    0
#define PID_OUTPUT_LIMIT          PWM_MAX_DUTY
#define PID_INTEGRAL_LIMIT        5000

/* PS2 摇杆原始值通常是 0-255，中位约 128。 */
#define PS2_NEUTRAL               128
#define PS2_AXIS_MAX              127

#endif
