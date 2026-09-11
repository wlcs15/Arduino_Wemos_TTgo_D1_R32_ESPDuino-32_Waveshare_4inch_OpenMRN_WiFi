#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
LIZARD="${LIZARD:-$(command -v lizard || true)}"
if [[ -z "$LIZARD" && -x "$HOME/.local/bin/lizard" ]]; then
    LIZARD="$HOME/.local/bin/lizard"
fi
if [[ -z "$LIZARD" ]]; then
    echo "lizard not on PATH" >&2
    exit 1
fi
out="$("$LIZARD" -C 10 "$ROOT/main" "$ROOT/tests" \
    --exclude "$ROOT/components/*" --exclude "$ROOT/managed_components/*" \
    --exclude "$ROOT/build/*" 2>&1)" || true
printf '%s\n' "$out"
if printf '%s\n' "$out" | grep -F '!!!! Warnings' >/dev/null; then
    echo "FAIL: lizard CCN limit 10 on main/ tests/" >&2
    exit 1
fi
echo "OK: lizard CCN limit 10 on main/ tests/"
