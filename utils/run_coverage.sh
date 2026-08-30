#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
clang++ -std=c++11 -fprofile-instr-generate -fcoverage-mapping \
    -I"$ROOT/main" "$ROOT/tests/test_svc_reach.cpp" -o /tmp/test_svc_reach_cov
LLVM_PROFILE_FILE=/tmp/test_svc_reach.profraw /tmp/test_svc_reach_cov
llvm-profdata merge -sparse /tmp/test_svc_reach.profraw -o /tmp/test_svc_reach.profdata
llvm-cov report /tmp/test_svc_reach_cov -instr-profile=/tmp/test_svc_reach.profdata \
    "$ROOT/main/SvcReachPick.h"
echo "coverage report above (SvcReachPick.h only; IDF main is not host-instrumented)"
