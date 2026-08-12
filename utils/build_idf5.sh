#!/usr/bin/env bash
# Build with ESP-IDF 5.1.6. No secret file is read.
#
# Password-free (Grok or any non-interactive shell):
#   ./utils/build_idf5.sh build
#   ./utils/build_idf5.sh -p /dev/ttyUSB0 flash
#
# Collect board IDs after a DEBUG flash, then provision in YOUR terminal:
#   ./utils/collect_hw_ids.py --port /dev/ttyUSB0
#   ./utils/provision_wifi_build.sh
#   ./utils/provision_wifi_build.sh -p /dev/ttyUSB0 flash
#
# Do not pass the Wi-Fi password on this command line.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [[ -z "${IDF_PATH:-}" ]]; then
    export IDF_PATH="${HOME}/esp/esp-idf-v5.1.6"
fi
# shellcheck disable=SC1091
source "${IDF_PATH}/export.sh"

# Same overlay CMake applies: add registry mdns to OpenMRNIDF REQUIRES.
PATCH="$ROOT/patches/OpenMRNIDF-idf51-mdns.patch"
if [[ -f "$PATCH" ]] && ! grep -qE '^[[:space:]]+mdns\)' "$ROOT/components/OpenMRNIDF/CMakeLists.txt"; then
    git -C "$ROOT/components/OpenMRNIDF" apply "$PATCH"
fi

if [[ $# -eq 0 ]]; then
    set -- build
fi

if [[ -f "$ROOT/main/wifi_psk_wrap.inc" ]]; then
    echo "Building with host-encrypted wrap blob (ciphertext only)."
else
    echo "Building without a wrap blob (DEBUG collect / NVS-only)."
fi

exec idf.py "$@"
