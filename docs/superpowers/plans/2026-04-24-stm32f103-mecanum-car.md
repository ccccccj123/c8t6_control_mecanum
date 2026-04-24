# STM32F103C8T6 麦轮小车 Keil 工程实施计划

> 这份计划记录本工程的实现步骤，后续如果要重构或迁移到别的开发板，可以按这里快速理解工程结构。

**目标：** 构建一个 STM32F103C8T6 麦轮小车 Keil 工程，支持 TB6612 电机驱动、PS2 手柄遥控、四轮编码器速度闭环和麦轮全向移动。

**架构：** 工程采用裸机寄存器方式，不依赖 HAL/SPL。核心算法与硬件驱动分离，麦轮运动学和 PID 可以在电脑上单独测试，STM32 外设初始化集中在 `Drivers/` 和 `Core/`。

**技术栈：** Keil uVision、ARMCC/ARMCLANG 可编译 C 代码、STM32F103C8T6 寄存器、少量主机端 GCC 测试。

---

## 文件结构

- `MecanumCar.uvprojx`：Keil 主工程文件。
- `MecanumCar.uvoptx`：Keil 工程选项文件。
- `Startup/startup_stm32f103c8tx.s`：启动文件、中断向量表、复位入口。
- `Core/stm32f103xb.h`：本工程需要的 STM32F103 最小寄存器定义。
- `Core/system_stm32f10x.c`：72MHz 系统时钟初始化。
- `Core/config.h`：速度、PWM、PID、手柄死区等调试参数。
- `Core/main.c`：初始化顺序和 10ms 主控制循环调度。
- `Drivers/gpio.*`：GPIO 时钟、模式配置、读写封装。
- `Drivers/pwm.*`：TIM1 四路 20kHz PWM。
- `Drivers/tb6612.*`：TB6612 方向控制和带符号 PWM 输出。
- `Drivers/encoder.*`：三路硬件编码器和一路 EXTI 软件编码器。
- `Drivers/ps2.*`：PS2 接收器 GPIO 位模拟通信。
- `Drivers/systick.*`：1ms 系统节拍。
- `App/mecanum.*`：麦轮运动学混控。
- `App/pid.*`：整数 PID 控制器。
- `App/control.*`：PS2、麦轮、编码器、PID、电机输出的整车控制逻辑。
- `docs/wiring.md`：中文接线和调车说明。
- `tests/test_mecanum_pid.c`：电脑端算法测试。
- `tools/verify_project.ps1`：静态工程检查脚本。

## 已完成任务

### 任务 1：核心算法测试

- 创建 `tests/test_mecanum_pid.c`。
- 覆盖前进、右平移、顺时针自转、混控限幅、PID 输出限幅、PID 积分复位。
- 先验证测试在没有实现时失败，再实现 `App/mecanum.c` 和 `App/pid.c`。

### 任务 2：STM32 裸机基础层

- 创建本地寄存器头 `Core/stm32f103xb.h`。
- 创建 72MHz HSE + PLL 系统时钟初始化。
- 创建启动文件和中断向量表。
- 保留 `SysTick`、`EXTI4`、`EXTI9_5` 中断入口。

### 任务 3：硬件驱动

- GPIO：支持输入、普通输出、复用推挽输出。
- PWM：使用 TIM1 CH1-CH4 输出 `PA8-PA11` 四路 20kHz PWM。
- TB6612：按四个轮子的引脚表输出方向和占空比。
- 编码器：TIM2/TIM3/TIM4 三路硬件编码器，PA4/PA5 一路 EXTI 软件正交解码。
- PS2：PB12-PB15 位模拟通信，读取按键和摇杆。
- SysTick：提供 1ms 节拍和简单延时。

### 任务 4：整车控制循环

- 每 10ms 读取 PS2 手柄。
- 左摇杆控制平移，右摇杆 X 轴控制自转。
- 麦轮运动学计算四轮目标速度。
- 四路编码器读取实际速度。
- 四个 PID 独立闭环。
- `START` 使能，`SELECT` 停车，`L1` 慢速。

### 任务 5：Keil 工程和文档

- 创建 Keil 工程文件。
- 写入中文接线说明。
- 创建静态验证脚本。
- 通过主机端测试、静态检查、XML 检查和 C 语法检查。

## 第四路硬件编码器的取舍

STM32F103C8T6 的定时器资源很紧。当前方案让 `TIM1` 负责四路硬件 PWM，让 `TIM2`、`TIM3`、`TIM4` 负责三路硬件编码器，最后一路编码器用 EXTI 软件计数。

如果改成四路硬件编码器，`TIM1` 就要拿去做编码器，四路 PWM 就没了。除非接受软件 PWM、外接 PWM 芯片，或者换定时器资源更多的芯片，否则当前方案是更适合这台小车的平衡点。

## 验证命令

```powershell
gcc tests/test_mecanum_pid.c App/mecanum.c App/pid.c -I. -o tests/test_mecanum_pid.exe
.\tests\test_mecanum_pid.exe
powershell -ExecutionPolicy Bypass -File tools/verify_project.ps1
gcc -fsyntax-only Core/system_stm32f10x.c Core/main.c Drivers/gpio.c Drivers/systick.c Drivers/pwm.c Drivers/tb6612.c Drivers/encoder.c Drivers/ps2.c App/mecanum.c App/pid.c App/control.c -ICore -IDrivers -IApp
```

期望结果：

- 测试输出 `All host tests passed`。
- 静态检查输出 `Static project verification passed.`。
- 语法检查无输出且退出码为 0。
