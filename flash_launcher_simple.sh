#!/bin/bash
# Simpler build and flash for game launcher from WSL

echo "========================================"
echo "Building Game Launcher"
echo "========================================"
echo ""

# Create a temporary batch file for Windows
TEMP_BAT="/mnt/c/temp_build_launcher.bat"

cat > "$TEMP_BAT" << 'EOF'
@echo off
cd /d C:\Users\lynn\Documents\Repositories\ebadge_ota\Apps\game_launcher
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1
C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe C:\Espressif\frameworks\esp-idf-v5.3.1\tools\idf.py build
EOF

# Run the batch file
cmd.exe /c "C:\\temp_build_launcher.bat"
BUILD_RESULT=$?

# Clean up
rm -f "$TEMP_BAT"

if [ $BUILD_RESULT -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Build Complete!"
echo "========================================"
echo ""

# Check if binary exists
if [ -f "/mnt/c/Users/lynn/Documents/Repositories/ebadge_ota/Apps/game_launcher/build/game_launcher.bin" ]; then
    SIZE=$(stat -c%s "/mnt/c/Users/lynn/Documents/Repositories/ebadge_ota/Apps/game_launcher/build/game_launcher.bin" 2>/dev/null)
    echo "Binary: game_launcher.bin ($SIZE bytes)"
    echo ""
fi

echo "Flashing to COM8..."
echo ""

# Create flash batch file
cat > "$TEMP_BAT" << 'EOF'
@echo off
cd /d C:\Users\lynn\Documents\Repositories\ebadge_ota\Apps\game_launcher
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1
C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe C:\Espressif\frameworks\esp-idf-v5.3.1\tools\idf.py -p COM8 flash
EOF

cmd.exe /c "C:\\temp_build_launcher.bat"
FLASH_RESULT=$?

rm -f "$TEMP_BAT"

if [ $FLASH_RESULT -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "Flash Complete!"
    echo "========================================"
    echo ""
    echo "Game Launcher is now on your eBadge!"
    echo "Press RESET to see the menu."
    echo ""
else
    echo ""
    echo "ERROR: Flash failed!"
    exit 1
fi
