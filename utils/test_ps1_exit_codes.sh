#!/usr/bin/env bash
# Copyright (c) 2027 Charles L. Sherman
# SPDX-License-Identifier: BSD-2-Clause
#
# Launcher for utils/test_ps1_exit_codes.ps1 (needs PowerShell 7: pwsh).
#   bash utils/test_ps1_exit_codes.sh        (PWSH=/path/to/pwsh to override)
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PWSH_BIN="${PWSH:-$(command -v pwsh || true)}"
if [ -z "$PWSH_BIN" ]; then
  echo "skip test_ps1_exit_codes (pwsh not on PATH; set PWSH=...)"
  exit 0
fi
exec "$PWSH_BIN" -NoProfile -File "$ROOT/utils/test_ps1_exit_codes.ps1"
