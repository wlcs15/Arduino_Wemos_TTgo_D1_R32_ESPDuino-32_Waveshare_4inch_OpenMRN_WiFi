# Host llvm-cov (same as utils/run_coverage.sh). Requires Clang.
# Reports SvcReachPick.h and ResetWhy.h only; IDF main is not host-instrumented.
#   powershell -NoProfile -ExecutionPolicy Bypass -File utils\run_coverage.ps1
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $Root

function Get-LlvmTool {
    param([string]$Name)
    $c = Get-Command $Name -ErrorAction SilentlyContinue
    if ($c) { return $c.Source }
    $p = Join-Path ${env:ProgramFiles} "LLVM\bin\$Name.exe"
    if (Test-Path $p) { return $p }
    throw "$Name not on PATH (install LLVM and add bin to PATH)"
}

$clang = Get-LlvmTool "clang++"
$profdata = Get-LlvmTool "llvm-profdata"
$cov = Get-LlvmTool "llvm-cov"
$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("d1r32_cov_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tmp | Out-Null
try {
    $svcBin = Join-Path $tmp "test_svc_reach_cov.exe"
    $whyBin = Join-Path $tmp "test_reset_why_cov.exe"
    $svcRaw = Join-Path $tmp "test_svc_reach.profraw"
    $whyRaw = Join-Path $tmp "test_reset_why.profraw"
    $svcData = Join-Path $tmp "test_svc_reach.profdata"
    $whyData = Join-Path $tmp "test_reset_why.profdata"

    & $clang -std=c++11 -D_CRT_SECURE_NO_WARNINGS -fprofile-instr-generate -fcoverage-mapping `
        "-I$Root\main" "$Root\tests\test_svc_reach.cpp" -o $svcBin
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $env:LLVM_PROFILE_FILE = $svcRaw
    & $svcBin
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    & $clang -std=c++11 -D_CRT_SECURE_NO_WARNINGS -fprofile-instr-generate -fcoverage-mapping `
        "-I$Root\main" "$Root\tests\test_reset_why.cpp" -o $whyBin
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $env:LLVM_PROFILE_FILE = $whyRaw
    & $whyBin
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Remove-Item Env:LLVM_PROFILE_FILE -ErrorAction SilentlyContinue

    & $profdata merge -sparse $svcRaw -o $svcData
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $profdata merge -sparse $whyRaw -o $whyData
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    & $cov report $svcBin "-instr-profile=$svcData" "$Root\main\SvcReachPick.h"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $cov report $whyBin "-instr-profile=$whyData" "$Root\main\ResetWhy.h"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Write-Host "coverage report above (host headers only; IDF main is not host-instrumented)"
} finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
