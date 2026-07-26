$tools = @(
    @{ Name = 'Git'; Command = 'git'; Hint = 'Required for version control.' },
    @{ Name = 'ZMK CLI'; Command = 'zmk'; Hint = 'Optional initially; install with: uv tool install zmk' },
    @{ Name = 'uv'; Command = 'uv'; Hint = 'Recommended Python/ZMK CLI package manager.' },
    @{ Name = 'West'; Command = 'west'; Hint = 'Needed only for local Zephyr builds.' },
    @{ Name = 'CMake'; Command = 'cmake'; Hint = 'Needed only for local Zephyr builds.' },
    @{ Name = 'nrfjprog'; Command = 'nrfjprog'; Hint = 'Optional J-Link/SWD recovery tool.' },
    @{ Name = 'J-Link Commander'; Command = 'JLinkExe'; Hint = 'Optional SWD debugging/recovery tool.' }
)

foreach ($tool in $tools) {
    $found = Get-Command $tool.Command -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host ("[FOUND] {0}: {1}" -f $tool.Name, $found.Source) -ForegroundColor Green
    } else {
        Write-Host ("[NOT FOUND] {0} - {1}" -f $tool.Name, $tool.Hint) -ForegroundColor Yellow
    }
}

Write-Host ''
Write-Host 'For the first firmware build, GitHub Actions is enough. A local Zephyr toolchain is not required.' -ForegroundColor Cyan

