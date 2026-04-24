$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$required = @(
    'MecanumCar.uvprojx',
    'MecanumCar.uvoptx',
    'Startup/startup_stm32f103c8tx.s',
    'Core/stm32f103xb.h',
    'Core/system_stm32f10x.c',
    'Core/config.h',
    'Core/main.c',
    'Drivers/gpio.c',
    'Drivers/pwm.c',
    'Drivers/tb6612.c',
    'Drivers/encoder.c',
    'Drivers/ps2.c',
    'Drivers/systick.c',
    'App/mecanum.c',
    'App/pid.c',
    'App/control.c',
    'docs/wiring.md'
)

foreach ($file in $required) {
    $path = Join-Path $root $file
    if (-not (Test-Path $path)) {
        throw "Missing required file: $file"
    }
}

$tb6612 = Get-Content (Join-Path $root 'Drivers/tb6612.c') -Raw
foreach ($pin in @('GPIOB, 0', 'GPIOB, 1', 'GPIOB, 10', 'GPIOB, 11', 'GPIOA, 12', 'GPIOA, 15', 'GPIOC, 13', 'GPIOC, 14')) {
    if ($tb6612 -notlike "*$pin*") {
        throw "TB6612 pin mapping missing $pin"
    }
}

$ps2 = Get-Content (Join-Path $root 'Drivers/ps2.c') -Raw
foreach ($pin in @('PS2_ATT_PIN  12', 'PS2_CLK_PIN  13', 'PS2_DAT_PIN  14', 'PS2_CMD_PIN  15')) {
    if ($ps2 -notlike "*$pin*") {
        throw "PS2 pin mapping missing $pin"
    }
}

$project = Get-Content (Join-Path $root 'MecanumCar.uvprojx') -Raw
foreach ($source in @('Core\main.c', 'Drivers\tb6612.c', 'Drivers\encoder.c', 'Drivers\ps2.c', 'App\control.c')) {
    if ($project -notlike "*$source*") {
        throw "Keil project does not include $source"
    }
}

Write-Host 'Static project verification passed.'
