#!/bin/bash
# Build and flash Game Launcher from WSL
# This uses Windows ESP-IDF tools from WSL

set -e

echo "============================================"
echo "Build and Flash Game Launcher (WSL)"
echo "============================================"
echo ""

# Convert WSL path to Windows path
PROJECT_DIR="/mnt/c/Users/lynn/Documents/Repositories/ebadge_ota"
LAUNCHER_DIR="$PROJECT_DIR/Apps/game_launcher"

# COM8 in Windows = /dev/ttyS8 in WSL
SERIAL_PORT="COM8"

echo "Step 1: Building Game Launcher..."
echo ""

cd "$LAUNCHER_DIR"

# Build using the actual Python environment that exists (3.11)
# Set required environment variables manually
cmd.exe /c "set IDF_PATH=C:\\Espressif\\frameworks\\esp-idf-v5.3.1 && set PATH=C:\\Espressif\\tools\\cmake\\3.24.0\\bin;C:\\Espressif\\tools\\ninja\\1.11.1;C:\\Espressif\\tools\\xtensa-esp-elf\\esp-13.2.0_20240530\\xtensa-esp-elf\\bin;C:\\Espressif\\tools\\riscv32-esp-elf\\esp-13.2.0_20240530\\riscv32-esp-elf\\bin;%PATH% && cd /d C:\\Users\\lynn\\Documents\\Repositories\\ebadge_ota\\Apps\\game_launcher && C:\\Espressif\\python_env\\idf5.3_py3.11_env\\Scripts\\python.exe C:\\Espressif\\frameworks\\esp-idf-v5.3.1\\tools\\idf.py build"

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "============================================"
echo "Build successful!"
echo "============================================"
echo ""

# Check if binary exists
if [ -f "build/game_launcher.bin" ]; then
    SIZE=$(stat -f%z "build/game_launcher.bin" 2>/dev/null || stat -c%s "build/game_launcher.bin" 2>/dev/null)
    echo "Binary: build/game_launcher.bin ($SIZE bytes)"
else
    echo "Warning: Binary not found at expected location"
fi

echo ""
echo "Step 2: Flashing to $SERIAL_PORT..."
echo ""

# Flash using the same environment setup
cmd.exe /c "set IDF_PATH=C:\\Espressif\\frameworks\\esp-idf-v5.3.1 && set PATH=C:\\Espressif\\tools\\cmake\\3.24.0\\bin;C:\\Espressif\\tools\\ninja\\1.11.1;C:\\Espressif\\tools\\xtensa-esp-elf\\esp-13.2.0_20240530\\xtensa-esp-elf\\bin;C:\\Espressif\\tools\\riscv32-esp-elf\\esp-13.2.0_20240530\\riscv32-esp-elf\\bin;%PATH% && cd /d C:\\Users\\lynn\\Documents\\Repositories\\ebadge_ota\\Apps\\game_launcher && C:\\Espressif\\python_env\\idf5.3_py3.11_env\\Scripts\\python.exe C:\\Espressif\\frameworks\\esp-idf-v5.3.1\\tools\\idf.py -p $SERIAL_PORT flash"

if [ $? -eq 0 ]; then
    echo ""
    echo "============================================"
    echo "Flash Complete!"
    echo "============================================"
    echo ""
    echo "Game Launcher is now running on your eBadge!"
    echo "Press RESET button to see the menu."
    echo ""
    echo "Controls:"
    echo "  UP/DOWN - Navigate menu"
    echo "  A Button - Launch game"
    echo ""
    read -p "Open serial monitor? (y/N): " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo ""
        echo "Opening monitor... Press CTRL+] to exit"
        echo ""
        cmd.exe /c "set IDF_PATH=C:\\Espressif\\frameworks\\esp-idf-v5.3.1 && cd /d C:\\Users\\lynn\\Documents\\Repositories\\ebadge_ota\\Apps\\game_launcher && C:\\Espressif\\python_env\\idf5.3_py3.11_env\\Scripts\\python.exe C:\\Espressif\\frameworks\\esp-idf-v5.3.1\\tools\\idf.py -p $SERIAL_PORT monitor"
    fi
else
    echo ""
    echo "ERROR: Flash failed!"
    echo ""
    echo "Troubleshooting:"
    echo "  1. Check USB cable is connected"
    echo "  2. Verify device is on COM8 (check Device Manager)"
    echo "  3. Close other serial programs"
    echo "  4. Try: sudo chmod 666 /dev/ttyS8"
    echo ""
    exit 1
fi

echo ""
echo "Done!"
