$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$requiredFiles = @(
    'README.md',
    '项目认知.md',
    '项目规划.md',
    '使用说明.md',
    'galpanel.log.md',
    '学习日志.md',
    '测试计划.md',
    'build.yaml',
    '.github/workflows/build.yml',
    'config/west.yml',
    'config/boards/shields/galpanel/galpanel.overlay',
    'config/boards/shields/galpanel/galpanel.keymap',
    'config/boards/shields/galpanel/galpanel.conf',
    'config/boards/shields/galpanel/galpanel_led_test.overlay',
    'config/boards/shields/galpanel/galpanel_led_test.keymap',
    'config/boards/shields/galpanel/galpanel_led_test.conf',
    'config/boards/shields/galpanel/galpanel_led_test.zmk.yml',
    'config/boards/shields/galpanel/galpanel_led_ind_test.overlay',
    'config/boards/shields/galpanel/galpanel_led_ind_test.keymap',
    'config/boards/shields/galpanel/galpanel_led_ind_test.conf',
    'config/boards/shields/galpanel/galpanel_led_ind_test.zmk.yml',
    'config/boards/shields/galpanel/galpanel_io_test.overlay',
    'config/boards/shields/galpanel/galpanel_io_test.keymap',
    'config/boards/shields/galpanel/galpanel_io_test.conf',
    'config/boards/shields/galpanel/galpanel_io_test.zmk.yml',
    'config/boards/shields/galpanel/galpanel_ec11_test.overlay',
    'config/boards/shields/galpanel/galpanel_ec11_test.keymap',
    'config/boards/shields/galpanel/galpanel_ec11_test.conf',
    'config/boards/shields/galpanel/galpanel_ec11_test.zmk.yml',
    'config/boards/shields/galpanel/galpanel_status_test.overlay',
    'config/boards/shields/galpanel/galpanel_status_test.conf',
    'config/boards/shields/galpanel/galpanel_status_test.zmk.yml',
    'config/boards/shields/galpanel/galpanel_safety_test.overlay',
    'config/boards/shields/galpanel/galpanel_safety_test.conf',
    'config/boards/shields/galpanel/galpanel_safety_test.zmk.yml',
    'config/boards/shields/galpanel/galpanel_win_ble_baseline.overlay',
    'config/boards/shields/galpanel/galpanel_win_ble_baseline.conf',
    'config/boards/shields/galpanel/galpanel_win_ble_baseline.keymap',
    'config/boards/shields/galpanel/galpanel_win_ble_baseline.zmk.yml',
    'src/galpanel_status.c',
    'src/galpanel_profile_name.c',
    'src/galpanel_win_ble_baseline.c',
    'config/galpanel_led_test.keymap',
    'config/galpanel_led_ind_test.keymap',
    'config/galpanel_io_test.keymap',
    'config/galpanel_ec11_test.keymap',
    'config/galpanel_status_test.keymap',
    'config/galpanel_safety_test.keymap',
    'config/galpanel_win_ble_baseline.keymap'
)

$failed = $false

foreach ($relativePath in $requiredFiles) {
    $fullPath = Join-Path $projectRoot $relativePath
    if (Test-Path -LiteralPath $fullPath -PathType Leaf) {
        Write-Host "[OK] $relativePath" -ForegroundColor Green
    } else {
        Write-Host "[MISSING] $relativePath" -ForegroundColor Red
        $failed = $true
    }
}

$checks = @(
    @{ File = 'build.yaml'; Pattern = 'nice_nano//zmk'; Name = 'nice!nano ZMK board target' },
    @{ File = 'build.yaml'; Pattern = 'shield: galpanel'; Name = 'GALPANEL shield target' },
    @{ File = 'build.yaml'; Pattern = 'shield: galpanel_led_test'; Name = 'GALPANEL LED test shield target' },
    @{ File = 'config/boards/shields/galpanel/galpanel.overlay'; Pattern = 'zmk,kscan-gpio-direct'; Name = 'direct GPIO key scan' },
    @{ File = 'config/boards/shields/galpanel/galpanel.overlay'; Pattern = 'steps = <80>'; Name = 'EC11 quadrature step count' },
    @{ File = 'config/boards/shields/galpanel/galpanel_led_test.overlay'; Pattern = 'output-high'; Name = 'LED test outputs are forced high' }
    @{ File = 'config/boards/shields/galpanel/galpanel_led_ind_test.overlay'; Pattern = 'zmk,indicator-leds'; Name = 'LED indicator test mapping' }
    @{ File = 'config/boards/shields/galpanel/galpanel_io_test.keymap'; Pattern = '&kp N9'; Name = 'IO test nine-key map' }
    @{ File = 'config/boards/shields/galpanel/galpanel_ec11_test.keymap'; Pattern = 'sensor-bindings'; Name = 'EC11 test sensor binding' },
    @{ File = 'config/galpanel_led_ind_test.keymap'; Pattern = 'galpanel_led_ind_test.keymap'; Name = 'LED test root keymap override' },
    @{ File = 'config/galpanel_io_test.keymap'; Pattern = 'galpanel_io_test.keymap'; Name = 'IO test root keymap override' },
    @{ File = 'config/galpanel_ec11_test.keymap'; Pattern = 'galpanel_ec11_test.keymap'; Name = 'EC11 test root keymap override' },
    @{ File = 'config/galpanel_status_test.keymap'; Pattern = 'galpanel.keymap'; Name = 'status test root keymap override' },
    @{ File = 'config/galpanel_safety_test.keymap'; Pattern = 'six-second destructive hold'; Name = 'safety test root keymap override' },
    @{ File = 'build.yaml'; Pattern = 'shield: galpanel_win_ble_baseline'; Name = 'Windows BLE baseline shield target' },
    @{ File = 'config/boards/shields/galpanel/galpanel_win_ble_baseline.conf'; Pattern = 'CONFIG_BT_GATT_ENFORCE_SUBSCRIPTION=n'; Name = 'Windows BLE baseline compatibility setting' },
    @{ File = 'config/galpanel_win_ble_baseline.keymap'; Pattern = 'galpanel_win_ble_baseline.keymap'; Name = 'Windows BLE baseline root keymap override' },
    @{ File = 'src/galpanel_win_ble_baseline.c'; Pattern = 'GP WIN BLE'; Name = 'Windows BLE baseline persistent-name override' }
)

foreach ($check in $checks) {
    $fullPath = Join-Path $projectRoot $check.File
    if ((Get-Content -Raw -LiteralPath $fullPath) -match [regex]::Escape($check.Pattern)) {
        Write-Host "[OK] $($check.Name)" -ForegroundColor Green
    } else {
        Write-Host "[FAILED] $($check.Name)" -ForegroundColor Red
        $failed = $true
    }
}

$conf = Get-Content -Raw -LiteralPath (Join-Path $projectRoot 'config/boards/shields/galpanel/galpanel.conf')
if ($conf -match 'CONFIG_ZMK_POINTING=y' -and $conf -match 'CONFIG_EC11=y') {
    Write-Host '[OK] Pointing and EC11 features are enabled' -ForegroundColor Green
} else {
    Write-Host '[FAILED] Pointing or EC11 feature is not enabled' -ForegroundColor Red
    $failed = $true
}

if ($failed) {
    throw 'GALPANEL project validation failed.'
}

Write-Host 'GALPANEL project structure validation passed.' -ForegroundColor Cyan
