#!/usr/bin/env bash
set -euo pipefail
PORT="${1:-${ESPPORT:-/dev/ttyS10}}"
if [ -z "${IDF_PATH:-}" ]; then
  if [ -f "$HOME/esp/esp-idf/export.sh" ]; then
    source "$HOME/esp/esp-idf/export.sh"
  elif [ -f "/mnt/c/Espressif/frameworks/esp-idf-v5.3.1/export.sh" ]; then
    source "/mnt/c/Espressif/frameworks/esp-idf-v5.3.1/export.sh"
  fi
fi
idf.py -p "$PORT" flash
