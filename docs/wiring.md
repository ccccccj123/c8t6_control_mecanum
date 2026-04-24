# STM32F103C8T6 Mecanum Car Wiring

## Board Pins

This project assumes the minimum system board exposes PB0-PB15, PA0-PA12, PA15, PC13-PC15, 5V, two 3V3 pins, three GND pins, VB, and R. Keep PA13 and PA14 free for ST-Link SWDIO/SWCLK.

## PS2 Receiver

| PS2 Signal | STM32 Pin |
| --- | --- |
| ATT / CS | PB12 |
| CLK | PB13 |
| DAT | PB14 |
| CMD | PB15 |
| VCC | 3V3 |
| GND | GND |

## TB6612 Motor Drivers

Use two TB6612 modules. Connect both STBY pins to PB2.

| Wheel | PWM | IN1 | IN2 | Driver Channel |
| --- | --- | --- | --- | --- |
| Front-left | PA8 | PB0 | PB1 | TB6612 #1 A |
| Front-right | PA9 | PB10 | PB11 | TB6612 #1 B |
| Rear-left | PA10 | PA12 | PA15 | TB6612 #2 A |
| Rear-right | PA11 | PC13 | PC14 | TB6612 #2 B |

TB6612 `VM` connects to the motor battery positive or board `VB` if that pin is your battery input. TB6612 `VCC` connects to 3V3 or 5V according to the exact module. All grounds must be common: STM32 GND, PS2 GND, TB6612 GND, battery negative, and encoder GND.

## Hall 370 Motor Encoders

| Wheel | Encoder A | Encoder B | Firmware Mode |
| --- | --- | --- | --- |
| Front-left | PA0 | PA1 | TIM2 encoder mode |
| Front-right | PA6 | PA7 | TIM3 encoder mode |
| Rear-left | PB6 | PB7 | TIM4 encoder mode |
| Rear-right | PA4 | PA5 | EXTI software quadrature |

If the encoder board is powered from 5V, verify its signal outputs are safe for STM32 input. Prefer open-drain outputs pulled up to 3V3, a level shifter, or a resistor divider when the output is push-pull 5V.

## Controls

- Press `START` on the PS2 controller to enable motor output.
- Press `SELECT` to stop motor output.
- Left joystick controls translation.
- Right joystick X axis controls rotation.
- Hold `L1` for slow mode.

No gyroscope is used, so the car cannot hold absolute heading. Rotation is commanded by wheel speed only.
