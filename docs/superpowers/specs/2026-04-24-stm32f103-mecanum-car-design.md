# STM32F103C8T6 Mecanum Car Keil Project Design

## Goal

Create a Keil uVision project for an STM32F103C8T6 minimum system board driving a four-wheel mecanum car with two TB6612 motor drivers, four Hall 370 geared motors, encoder speed closed-loop control, and PS2 controller remote control. The car has no gyroscope, so motion control is open-loop in chassis heading and closed-loop only in wheel speed.

## Hardware Pin Map

Available board pins are PB0-PB15, PA0-PA12, PA15, PC13-PC15, 5V, two 3V3 pins, three GND pins, VB, and R. PA13 and PA14 are reserved for SWD download/debug.

### PS2 Receiver

| Signal | STM32 Pin |
| --- | --- |
| ATT / CS | PB12 |
| CLK | PB13 |
| DAT | PB14 |
| CMD | PB15 |
| VCC | 3V3 |
| GND | GND |

### TB6612 Motor Control

| Wheel | PWM | IN1 | IN2 | Driver Channel |
| --- | --- | --- | --- | --- |
| Front-left | PA8 | PB0 | PB1 | TB6612 #1 A |
| Front-right | PA9 | PB10 | PB11 | TB6612 #1 B |
| Rear-left | PA10 | PA12 | PA15 | TB6612 #2 A |
| Rear-right | PA11 | PC13 | PC14 | TB6612 #2 B |

Both TB6612 STBY pins connect to PB2. TB6612 VM connects to the motor battery or VB, TB6612 logic VCC connects to 3V3 or 5V according to the module, and all grounds must be common.

### Hall Encoder Inputs

| Wheel | A Phase | B Phase | Implementation |
| --- | --- | --- | --- |
| Front-left | PA0 | PA1 | TIM2 encoder mode |
| Front-right | PA6 | PA7 | TIM3 encoder mode |
| Rear-left | PB6 | PB7 | TIM4 encoder mode |
| Rear-right | PA4 | PA5 | EXTI software quadrature count |

If encoder outputs are powered from 5V, confirm that the output level is safe for STM32 GPIO. Use open-drain pull-up to 3V3 or level shifting when needed.

## Firmware Architecture

The project uses bare-metal STM32F10x register access instead of SPL or HAL, so it can build in Keil without downloading a firmware library. CMSIS-style device definitions live in local headers. Startup code, system clock setup, drivers, control code, and application logic are split into small modules.

The main loop reads PS2 input, converts joystick commands into chassis velocity commands, computes four wheel speed targets with mecanum kinematics, then runs wheel speed PID at a 10 ms cadence. TIM1 provides four hardware PWM channels. TIM2, TIM3, and TIM4 count three encoders in hardware, and EXTI for PA4/PA5 counts the fourth encoder in software.

## Control Behavior

The PS2 left joystick controls X/Y translation. The right joystick X axis controls rotation. Without a gyroscope, rotation is relative to robot heading only and cannot hold absolute yaw. Button handling includes stop/enable behavior and speed scaling constants that can be tuned in `config.h`.

Wheel speed PID computes signed PWM commands from target encoder ticks per control cycle and measured ticks per cycle. TB6612 direction pins carry the sign, and TIM1 duty carries the magnitude.

## Project Outputs

The repository will contain:

- `MecanumCar.uvprojx` and `MecanumCar.uvoptx` for Keil.
- `Core/`, `Drivers/`, `App/`, and `Startup/` firmware sources.
- `docs/wiring.md` with the pin map and wiring notes.
- `tests/` host-side tests for mecanum kinematics and PID behavior.

## Verification

Host-side C tests compile with GCC if available and verify core math. Static checks confirm expected project files and key pin macros exist. Final MCU verification still requires opening the project in Keil after installation and flashing the STM32F103C8T6 board.
