#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
PY="${PYTHON:-python3}"
echo "== wrap tests =="
"$ROOT/utils/test_wifi_wrap.sh"
echo "== SvcReachPick =="
clang++ -std=c++11 -I"$ROOT/main" "$ROOT/tests/test_svc_reach.cpp" -o /tmp/test_svc_reach
/tmp/test_svc_reach
echo "== CDI Configure =="
clang++ -std=c++11 -I"$ROOT/tests" "$ROOT/tests/test_cdi_configure.cpp" -o /tmp/test_cdi_configure
/tmp/test_cdi_configure
echo "host tests OK"
