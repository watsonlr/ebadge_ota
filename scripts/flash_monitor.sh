#!/usr/bin/env bash
set -euo pipefail
PORT="${1:-${ESPPORT:-/dev/ttyUSB0}}"
source "${IDF_PATH:-$HOME/esp/esp-idf}/export.sh" 2>/dev/null || true
idf.py -p "$PORT" flash monitor
