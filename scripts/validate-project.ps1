$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$requiredFiles = @(
    'README.md',
    '项目认知.md',
    '项目规划.md',
    'galpanel.log.md',
    '学习日志.md',
    'build.yaml',
    '.github/workflows/build.yml',
    'config/west.yml',
    'config/boards/shields/galpanel/galpanel.overlay',
    'config/boards/shields/galpanel/galpanel.keymap',
    'config/boards/shields/galpanel/galpanel.conf'
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
    @{ File = 'config/boards/shields/galpanel/galpanel.overlay'; Pattern = 'zmk,kscan-gpio-direct'; Name = 'direct GPIO key scan' },
    @{ File = 'config/boards/shields/galpanel/galpanel.overlay'; Pattern = 'steps = <80>'; Name = 'EC11 quadrature step count' }
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
