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

/* 速度闭环周期。10ms 对霍尔 520 减速电机比较稳，太短会让低速计数抖动明显。 */
#define CONTROL_PERIOD_MS         10U

/* TB6612 PWM 频率。20kHz 基本避开人耳可闻范围。 */
#define PWM_FREQUENCY_HZ          20000U

/* PWM 占空比软件量程，1000 表示 100%。 */
#define PWM_MAX_DUTY              1000

/*
 * 电机和编码器参数。
 *
 * 当前电机：12V 333rpm 520 霍尔编码器减速电机。
 * 编码器基础脉冲：11PPR。
 * 减速比：30。
 *
 * 注意：这里的 11PPR * 30 = 330，是输出轴每圈每相的基础脉冲数。
 * 本工程硬件编码器和 EXTI 软件编码器都按四倍频计数，
 * 所以程序里看到的一圈计数约为 11 * 30 * 4 = 1320。
 *
 * 333rpm 空载时：
 * 333 / 60 = 5.55 转/秒
 * 5.55 * 1320 = 7326 计数/秒
 * 10ms 控制周期内约 73 计数
 *
 * 因此 SPEED_LIMIT_TICKS 不能再用几百，否则 PID 会一直追一个电机达不到的速度。
 */
#define MOTOR_NO_LOAD_RPM         333
#define ENCODER_BASE_PPR          11
#define MOTOR_GEAR_RATIO          30
#define ENCODER_QUAD_MULTIPLIER   4
#define ENCODER_COUNTS_PER_REV    (ENCODER_BASE_PPR * MOTOR_GEAR_RATIO * ENCODER_QUAD_MULTIPLIER)
#define MOTOR_MAX_TICKS_10MS      ((MOTOR_NO_LOAD_RPM * ENCODER_COUNTS_PER_REV * CONTROL_PERIOD_MS) / 60000U)

/*
 * 普通/慢速模式下的单轮目标速度上限，按编码器每 10ms 的计数增量计算。
 *
 * 调车建议：
 * 1. 第一次上地前先把 SPEED_LIMIT_TICKS 调小，例如 25-40；
 * 2. 确认四轮方向、编码器方向都正确后再慢慢加大；
 * 3. 如果车一给油就抖，先降低 SPEED_LIMIT_TICKS 和 PID_KP。
 *
 * MOTOR_MAX_TICKS_10MS 只是空载理论值，真实落地会更低。
 * 初学者调试建议先用最大值的一半左右，也就是 35 左右。
 */
#define SPEED_LIMIT_TICKS         35
#define SLOW_SPEED_TICKS          18

/*
 * 上电后的 LED 心跳周期。
 * 如果只拿一块 C8T6 最小系统板测试，烧录后看到 PC13 LED 闪烁，
 * 就说明时钟、SysTick、GPIO 输出和主循环都已经在工作。
 */
#define LED_HEARTBEAT_MS          500U

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
#define PID_KP                    12
#define PID_KI                    1
#define PID_KD                    0
#define PID_OUTPUT_LIMIT          PWM_MAX_DUTY
#define PID_INTEGRAL_LIMIT        5000

/* PS2 摇杆原始值通常是 0-255，中位约 128。 */
#define PS2_NEUTRAL               128
#define PS2_AXIS_MAX              127

/* OLED 使用软件 I2C。PB8/PB9 目前没有被电机、编码器、PS2 占用。 */
#define OLED_REFRESH_MS           200U

#endif
