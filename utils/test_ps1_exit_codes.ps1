# Copyright (c) 2027 Charles L. Sherman
# SPDX-License-Identifier: BSD-2-Clause
#
# Regression test: utils/test_wifi_wrap.ps1 must leave exit code 0 on success.
# run_host_tests.ps1 calls it with `& script` and then checks $LASTEXITCODE;
# a stale non-zero code (e.g. from the provisioner non-TTY refusal check)
# used to abort run_host_tests.ps1 before the clang tests ran.
#   pwsh -NoProfile -File utils/test_ps1_exit_codes.ps1
#   powershell -NoProfile -ExecutionPolicy Bypass -File utils\test_ps1_exit_codes.ps1
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Target = Join-Path $PSScriptRoot "test_wifi_wrap.ps1"
$Shell = (Get-Process -Id $PID).Path
$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("d1r32_exitcodes_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tmp | Out-Null
$oldPath = $env:PATH
$failed = 0
try {
    # Off Windows there is no cmd.exe; test_wifi_wrap.ps1 uses `cmd /c` for its
    # provisioner TTY check. Add a tiny shim so the same native call (and the
    # same non-zero $LASTEXITCODE it leaves behind) happens on Linux/macOS.
    $isWin = ($env:OS -eq "Windows_NT")
    if (-not $isWin -and -not (Get-Command cmd -ErrorAction SilentlyContinue)) {
        $shim = Join-Path $tmp "cmd"
        $shimText = @'
#!/bin/sh
# test shim: emulate `cmd /c "<line>"` (type NUL / >NUL) for test_wifi_wrap.ps1
[ "$1" = "/c" ] && shift
line=$(printf '%s' "$*" | sed -e 's#type NUL#cat /dev/null#g' -e 's#>NUL#>/dev/null#g')
exec sh -c "$line"
'@
        [System.IO.File]::WriteAllText($shim, ($shimText -replace "`r", ""))
        & chmod +x $shim
        $env:PATH = $tmp + [System.IO.Path]::PathSeparator + $env:PATH
        Write-Host "(using cmd shim at $shim)"
    }

    Write-Host "== case 1: child -File exit code =="
    & $Shell -NoProfile -ExecutionPolicy Bypass -File $Target | Out-Null
    $code1 = $LASTEXITCODE
    if ($code1 -eq 0) { Write-Host "PASS: child exit code 0" }
    else { Write-Host "FAIL: child exit code $code1 (expected 0)" -ForegroundColor Red; $failed++ }

    Write-Host "== case 2: '& script' then `$LASTEXITCODE (as run_host_tests.ps1) =="
    # Start with a stale non-zero code from a native command, then call the
    # script exactly like run_host_tests.ps1 does.
    $harness = Join-Path $tmp "harness.ps1"
    $h = @(
        '$ErrorActionPreference = "Stop"',
        'if (Get-Command bash -ErrorAction SilentlyContinue) { bash -c "exit 7" } else { cmd /c "exit 7" }',
        ('& "' + $Target + '" | Out-Null'),
        'Write-Output ("LASTEXITCODE=" + $LASTEXITCODE)',
        'exit $LASTEXITCODE'
    )
    Set-Content -Path $harness -Value $h
    $out = & $Shell -NoProfile -ExecutionPolicy Bypass -File $harness
    $code2 = $LASTEXITCODE
    $out | Where-Object { $_ -match 'LASTEXITCODE=' } | ForEach-Object { Write-Host $_ }
    if ($code2 -eq 0) { Write-Host "PASS: `$LASTEXITCODE 0 after '& test_wifi_wrap.ps1'" }
    else { Write-Host "FAIL: `$LASTEXITCODE $code2 after '& test_wifi_wrap.ps1' (expected 0)" -ForegroundColor Red; $failed++ }
} finally {
    $env:PATH = $oldPath
    Set-Location $Root
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
if ($failed -ne 0) {
    Write-Host "ps1 exit-code tests FAILED ($failed)" -ForegroundColor Red
    exit 1
}
Write-Host "ps1 exit-code tests passed."
exit 0
