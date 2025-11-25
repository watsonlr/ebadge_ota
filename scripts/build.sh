#!/usr/bin/env bash
set -euo pipefail
source "${IDF_PATH:-$HOME/esp/esp-idf}/export.sh" 2>/dev/null || true
idf.py set-target esp32c3 build
