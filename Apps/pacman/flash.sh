#!/bin/bash
# Flash Pac-Man game to ESP32-S3

# Source ESP-IDF
if [ -f "$HOME/esp/esp-idf/export.sh" ]; then
    source "$HOME/esp/esp-idf/export.sh"
elif [ -f "/Users/$USER/esp/esp-idf/export.sh" ]; then
    source "/Users/$USER/esp/esp-idf/export.sh"
else
    echo "ESP-IDF not found!"
    exit 1
fi

# Set port (default or from environment)
PORT="${ESPPORT:-/dev/ttyS10}"

echo "Flashing Pac-Man game to $PORT..."
echo ""

idf.py -p $PORT flash monitor
