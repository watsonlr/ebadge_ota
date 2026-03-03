#!/bin/bash
# Build Tetris game

# Source ESP-IDF
if [ -f "$HOME/esp/esp-idf/export.sh" ]; then
    source "$HOME/esp/esp-idf/export.sh"
elif [ -f "/Users/$USER/esp/esp-idf/export.sh" ]; then
    source "/Users/$USER/esp/esp-idf/export.sh"
else
    echo "ESP-IDF not found! Please install ESP-IDF first."
    exit 1
fi

echo "Building Tetris game..."
idf.py build

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Build successful!"
    echo "Binary: build/tetris_game.bin"
    echo ""
    echo "To flash: ./flash.sh"
else
    echo "✗ Build failed"
    exit 1
fi
