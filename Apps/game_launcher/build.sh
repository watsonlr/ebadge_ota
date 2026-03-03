#!/bin/bash
# Build script for game launcher

echo "Building Game Launcher..."
idf.py build

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Binary: build/game_launcher.bin"
else
    echo "Build failed!"
    exit 1
fi
