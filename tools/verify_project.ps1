$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$required = @(
    'c8t6_control_mecanum.uvprojx',
    'c8t6_control_mecanum.uvoptx',
    'Startup/startup_stm32f103c8tx.s',
    'Core/stm32f103xb.h',
    'Core/system_stm32f10x.c',
    'Core/config.h',
    'Core/main.c',
    'Drivers/gpio.c',
    'Drivers/led.c',
    'Drivers/oled.c',
    'Drivers/pwm.c',
    'Drivers/tb6612.c',
    'Drivers/encoder.c',
    'Drivers/ps2.c',
    'Drivers/systick.c',
    'App/mecanum.c',
    'App/pid.c',
    'App/control.c',
    'docs/wiring.md',
    'docs/debug_flow.md'
)

foreach ($file in $required) {
    $path = Join-Path $root $file
    if (-not (Test-Path $path)) {
        throw "Missing required file: $file"
    }
}

$tb6612 = Get-Content (Join-Path $root 'Drivers/tb6612.c') -Raw
foreach ($pin in @('GPIOB, 0', 'GPIOB, 1', 'GPIOB, 10', 'GPIOB, 11', 'GPIOA, 12', 'GPIOA, 15', 'GPIOB, 4', 'GPIOB, 5')) {
    if ($tb6612 -notlike "*$pin*") {
        throw "TB6612 pin mapping missing $pin"
    }
}
if ($tb6612 -like '*GPIOC, 13*') {
    throw 'PC13 must stay reserved for the board LED, but TB6612 still references GPIOC, 13'
}

$led = Get-Content (Join-Path $root 'Drivers/led.c') -Raw
foreach ($pin in @('LED_PORT GPIOC', 'LED_PIN  13')) {
    if ($led -notlike "*$pin*") {
        throw "LED pin mapping missing $pin"
    }
}

$oled = Get-Content (Join-Path $root 'Drivers/oled.c') -Raw
foreach ($pin in @('OLED_SCL_PIN  8', 'OLED_SDA_PIN  9')) {
    if ($oled -notlike "*$pin*") {
        throw "OLED pin mapping missing $pin"
    }
}

$ps2 = Get-Content (Join-Path $root 'Drivers/ps2.c') -Raw
foreach ($pin in @('PS2_ATT_PIN  12', 'PS2_CLK_PIN  13', 'PS2_DAT_PIN  14', 'PS2_CMD_PIN  15')) {
    if ($ps2 -notlike "*$pin*") {
        throw "PS2 pin mapping missing $pin"
    }
}

$project = Get-Content (Join-Path $root 'c8t6_control_mecanum.uvprojx') -Raw
foreach ($source in @('Core\main.c', 'Drivers\led.c', 'Drivers\oled.c', 'Drivers\tb6612.c', 'Drivers\encoder.c', 'Drivers\ps2.c', 'App\control.c')) {
    if ($project -notlike "*$source*") {
        throw "Keil project does not include $source"
    }
}

Write-Host 'Static project verification passed.'
