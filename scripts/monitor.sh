#!/usr/bin/env bash
# Simple wrapper to monitor device without mixing environments.
# Usage: ./scripts/monitor.sh [/dev/ttyS10]
set -euo pipefail
PORT="${1:-/dev/ttyS10}"
exec idf.py -p "$PORT" monitor
