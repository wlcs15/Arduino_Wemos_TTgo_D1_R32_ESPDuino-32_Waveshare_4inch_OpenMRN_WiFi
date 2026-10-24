#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
clang++ -std=c++11 -fprofile-instr-generate -fcoverage-mapping \
    -I"$ROOT/main" "$ROOT/tests/test_svc_reach.cpp" -o /tmp/test_svc_reach_cov
LLVM_PROFILE_FILE=/tmp/test_svc_reach.profraw /tmp/test_svc_reach_cov
clang++ -std=c++11 -fprofile-instr-generate -fcoverage-mapping \
    -I"$ROOT/main" "$ROOT/tests/test_reset_why.cpp" -o /tmp/test_reset_why_cov
LLVM_PROFILE_FILE=/tmp/test_reset_why.profraw /tmp/test_reset_why_cov
llvm-profdata merge -sparse /tmp/test_svc_reach.profraw -o /tmp/test_svc_reach.profdata
llvm-profdata merge -sparse /tmp/test_reset_why.profraw -o /tmp/test_reset_why.profdata
llvm-cov report /tmp/test_svc_reach_cov -instr-profile=/tmp/test_svc_reach.profdata \
    "$ROOT/main/SvcReachPick.h"
llvm-cov report /tmp/test_reset_why_cov -instr-profile=/tmp/test_reset_why.profdata \
    "$ROOT/main/ResetWhy.h"
echo "coverage report above (host headers only; IDF main is not host-instrumented)"
