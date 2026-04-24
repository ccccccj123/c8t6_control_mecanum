# STM32F103 Mecanum Car Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Keil uVision project for an STM32F103C8T6 mecanum car with TB6612 drivers, PS2 remote input, and four-wheel encoder speed PID.

**Architecture:** The firmware is a bare-metal register-level STM32F10x project so it does not depend on HAL/SPL downloads. Core math is isolated from hardware drivers for host-side testing, while hardware modules handle GPIO, PWM, encoder, EXTI, SysTick, PS2 bit-bang communication, and TB6612 output.

**Tech Stack:** Keil uVision project files, ARMCC-style embedded C, CMSIS-compatible local startup/header files, and optional GCC host tests for non-hardware logic.

---

## File Structure

- Create `MecanumCar.uvprojx` and `MecanumCar.uvoptx`: Keil project metadata.
- Create `Startup/startup_stm32f103c8tx.s`: vector table and reset entry.
- Create `Core/stm32f103xb.h`, `Core/system_stm32f10x.c`, `Core/main.c`, `Core/config.h`: device definitions, clock setup, application loop, tunables.
- Create `Drivers/gpio.*`, `Drivers/pwm.*`, `Drivers/tb6612.*`, `Drivers/encoder.*`, `Drivers/ps2.*`, `Drivers/systick.*`: hardware access modules.
- Create `App/pid.*`, `App/mecanum.*`, `App/control.*`: behavior and control logic.
- Create `docs/wiring.md`: final pin map and safety notes.
- Create `tests/test_mecanum_pid.c`: host-side tests for core math.
- Create `tools/verify_project.ps1`: static project verification.

### Task 1: Add Core Math Tests

**Files:**
- Create: `tests/test_mecanum_pid.c`
- Create: `App/mecanum.h`
- Create: `App/pid.h`

- [ ] **Step 1: Write failing host tests**

Create tests that include `App/mecanum.h` and `App/pid.h`, then assert forward/strafe/rotate wheel target signs and PID output clamping.

- [ ] **Step 2: Run test to verify it fails**

Run: `gcc tests/test_mecanum_pid.c -I. -o tests/test_mecanum_pid.exe`

Expected: FAIL because implementation files do not exist yet.

- [ ] **Step 3: Implement math modules**

Create `App/mecanum.c` and `App/pid.c` with signed wheel mixing and PID update/reset.

- [ ] **Step 4: Run tests to verify pass**

Run: `gcc tests/test_mecanum_pid.c App/mecanum.c App/pid.c -I. -o tests/test_mecanum_pid.exe; ./tests/test_mecanum_pid.exe`

Expected: PASS with `All host tests passed`.

### Task 2: Add Bare-Metal STM32 Foundation

**Files:**
- Create: `Core/stm32f103xb.h`
- Create: `Core/system_stm32f10x.c`
- Create: `Core/config.h`
- Create: `Startup/startup_stm32f103c8tx.s`

- [ ] **Step 1: Add local register definitions**

Define peripheral base addresses, register layouts, GPIO bit helpers, RCC, AFIO, EXTI, NVIC, SysTick, TIM, and SPI-style GPIO constants needed by the project.

- [ ] **Step 2: Add startup and clock setup**

Use an 8 MHz HSE to configure 72 MHz SYSCLK. Provide vector entries for SysTick, EXTI4, EXTI9_5, and timer interrupts used by the firmware.

### Task 3: Add Hardware Drivers

**Files:**
- Create: `Drivers/gpio.h`, `Drivers/gpio.c`
- Create: `Drivers/pwm.h`, `Drivers/pwm.c`
- Create: `Drivers/tb6612.h`, `Drivers/tb6612.c`
- Create: `Drivers/encoder.h`, `Drivers/encoder.c`
- Create: `Drivers/ps2.h`, `Drivers/ps2.c`
- Create: `Drivers/systick.h`, `Drivers/systick.c`

- [ ] **Step 1: Implement GPIO and SysTick helpers**

Configure GPIO modes and provide millisecond tick timing.

- [ ] **Step 2: Implement PWM and motor output**

Set TIM1 CH1-CH4 on PA8-PA11 to 20 kHz PWM and map signed motor commands to TB6612 direction pins and duty values.

- [ ] **Step 3: Implement encoders**

Set TIM2, TIM3, and TIM4 encoder mode. Decode PA4/PA5 with EXTI for the rear-right encoder.

- [ ] **Step 4: Implement PS2 bit-bang input**

Use PB12-PB15 to poll the PS2 receiver and expose joystick/button state.

### Task 4: Add Application Control Loop

**Files:**
- Create: `App/control.h`, `App/control.c`
- Create: `Core/main.c`

- [ ] **Step 1: Implement control loop**

Every 10 ms, read PS2, normalize joystick commands, compute wheel targets, read encoder deltas, update four PIDs, and write motor commands.

- [ ] **Step 2: Implement startup path**

Initialize clock, SysTick, GPIO, PWM, TB6612, encoders, PS2, and control state in `main`.

### Task 5: Add Keil Project and Docs

**Files:**
- Create: `MecanumCar.uvprojx`
- Create: `MecanumCar.uvoptx`
- Create: `docs/wiring.md`
- Create: `tools/verify_project.ps1`

- [ ] **Step 1: Create Keil project**

Reference all source files and configure target for STM32F103C8, 64 KB flash, 20 KB RAM.

- [ ] **Step 2: Write wiring document**

Document PS2, TB6612, encoder, power, and download/debug pin usage.

- [ ] **Step 3: Add static verifier**

Check that all expected files exist and that important pin macros appear in source.

- [ ] **Step 4: Run verification**

Run host tests if GCC is installed and run `powershell -ExecutionPolicy Bypass -File tools/verify_project.ps1`.

Expected: static verifier passes; host tests pass when GCC is available.

## Self-Review

The plan covers the approved pin map, PS2 input, TB6612 output, four encoders, mecanum mixing, PID, Keil project generation, wiring docs, and verification. No placeholder implementation steps remain.
