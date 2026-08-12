#!/usr/bin/env bash
# Build with ESP-IDF 5.1.6. Wi-Fi secrets come from wifi_secrets.env only.
# Values are never printed. Do not pass SSID/password on the command line.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

SECRETS="$ROOT/wifi_secrets.env"
if [[ ! -f "$SECRETS" ]]; then
    echo "Missing $SECRETS"
    echo "Copy wifi_secrets.env.example to wifi_secrets.env and edit it locally."
    exit 1
fi

set -a
# shellcheck disable=SC1090
source "$SECRETS"
set +a

if [[ -z "${WIFI_SSID:-}" || -z "${WIFI_PASSWORD:-}" ]]; then
    echo "WIFI_SSID and WIFI_PASSWORD must be non-empty in wifi_secrets.env"
    exit 1
fi

# Do not echo SSID or password.
export WIFI_SSID WIFI_PASSWORD

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

exec idf.py "$@"
