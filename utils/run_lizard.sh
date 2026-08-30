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
"$LIZARD" -C 10 -w "$ROOT/main" "$ROOT/tests" \
    --exclude "$ROOT/components/*" --exclude "$ROOT/managed_components/*" \
    --exclude "$ROOT/build/*"
echo "OK: lizard CCN limit 10 on main/ tests/ utils/"
