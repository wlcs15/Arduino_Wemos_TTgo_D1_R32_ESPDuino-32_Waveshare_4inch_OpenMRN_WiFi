# Cyclomatic complexity (same as utils/run_lizard.sh).
# Gate: CCN 10 on main/ and tests/. Optional LIZARD_ALL=1 is unused here (gate is already main+tests).
#   powershell -NoProfile -ExecutionPolicy Bypass -File utils\run_lizard.ps1
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $Root

function Invoke-Lizard {
    param([string[]]$LizardArgs)
    $exe = Get-Command lizard -ErrorAction SilentlyContinue
    if ($exe) {
        & $exe.Source @LizardArgs
        return $LASTEXITCODE
    }
    foreach ($pair in @(
        @{ Exe = "python"; Prefix = @() },
        @{ Exe = "python3"; Prefix = @() },
        @{ Exe = "py"; Prefix = @("-3") }
    )) {
        if (-not (Get-Command $pair.Exe -ErrorAction SilentlyContinue)) { continue }
        $null = & $pair.Exe @($pair.Prefix + @("-c", "import lizard")) 2>$null
        if ($LASTEXITCODE -eq 0) {
            & $pair.Exe @($pair.Prefix + @("-m", "lizard") + $LizardArgs)
            return $LASTEXITCODE
        }
    }
    $pipx = Join-Path $env:LOCALAPPDATA "pipx\venvs\lizard\Scripts\lizard.exe"
    if (Test-Path $pipx) {
        & $pipx @LizardArgs
        return $LASTEXITCODE
    }
    throw "lizard not on PATH (pipx install lizard, or python -m pip install lizard)"
}

$gateArgs = @(
    "-C", "10",
    (Join-Path $Root "main"),
    (Join-Path $Root "tests"),
    "--exclude", (Join-Path $Root "components\*"),
    "--exclude", (Join-Path $Root "managed_components\*"),
    "--exclude", (Join-Path $Root "build\*")
)
$ErrorActionPreference = "Continue"
$gateOut = Invoke-Lizard $gateArgs 2>&1 | Out-String
$ErrorActionPreference = "Stop"
Write-Host $gateOut.TrimEnd()
if ($gateOut -match '!!!! Warnings') {
    Write-Error "FAIL: lizard CCN limit 10 on main/ tests/"
    exit 1
}
Write-Host "OK: lizard CCN limit 10 on main/ tests/"
exit 0
