#!/bin/bash
# Build all games and launcher for ESP32-S3 eBadge
# Run this from a terminal with ESP-IDF environment sourced

set -e  # Exit on error

echo "============================================"
echo "Building All eBadge Apps"
echo "============================================"
echo ""

# Check if ESP-IDF is available
if ! command -v idf.py &> /dev/null; then
    echo "ERROR: idf.py not found!"
    echo "Please source ESP-IDF environment first:"
    echo "  source \$HOME/esp/esp-idf/export.sh"
    echo "or"
    echo "  . \$IDF_PATH/export.sh"
    exit 1
fi

# Save original directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build Pac-Man
echo "[1/4] Building Pac-Man..."
cd "$SCRIPT_DIR/Apps/pacman"
idf.py build
echo "✓ Pac-Man build successful!"
echo ""

# Build Tetris
echo "[2/4] Building Tetris..."
cd "$SCRIPT_DIR/Apps/tetris"
idf.py build
echo "✓ Tetris build successful!"
echo ""

# Build Frogger
echo "[3/4] Building Frogger..."
cd "$SCRIPT_DIR/Apps/frogger"
idf.py build
echo "✓ Frogger build successful!"
echo ""

# Build Game Launcher
echo "[4/4] Building Game Launcher..."
cd "$SCRIPT_DIR/Apps/game_launcher"
idf.py build
echo "✓ Game Launcher build successful!"
echo ""

# Copy binaries to OTA directory
echo "============================================"
echo "Copying binaries to OTA directory..."
echo "============================================"

cd "$SCRIPT_DIR"

mkdir -p ota_files/apps

cp Apps/pacman/build/pacman_game.bin ota_files/apps/pacman.bin
cp Apps/tetris/build/tetris_game.bin ota_files/apps/tetris.bin
cp Apps/frogger/build/frogger_game.bin ota_files/apps/frogger.bin
cp Apps/game_launcher/build/game_launcher.bin ota_files/apps/launcher.bin

echo ""
echo "============================================"
echo "Build Complete!"
echo "============================================"
echo ""
echo "Binaries copied to ota_files/apps/:"
echo "  ✓ pacman.bin"
echo "  ✓ tetris.bin"
echo "  ✓ frogger.bin"
echo "  ✓ launcher.bin"
echo ""
echo "To flash the game launcher:"
echo "  cd Apps/game_launcher"
echo "  idf.py -p /dev/ttyUSB0 flash"
echo ""
echo "To start OTA server:"
echo "  python3 simple_ota_server.py"
echo ""
