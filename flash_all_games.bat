@echo off
echo ========================================
echo Flashing Game Launcher + All 3 Games
echo ========================================

set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1
set PATH=C:\Espressif\tools\cmake\3.24.0\bin;C:\Espressif\tools\ninja\1.11.1;C:\Espressif\tools\xtensa-esp-elf\esp-13.2.0_20240530\xtensa-esp-elf\bin;C:\Espressif\tools\riscv32-esp-elf\esp-13.2.0_20240530\riscv32-esp-elf\bin;C:\Espressif\python_env\idf5.3_py3.11_env\Scripts;%PATH%

echo.
echo [1/4] Building and Flashing Launcher (Factory partition)...
cd /d "%~dp0Apps\game_launcher"
python %IDF_PATH%\tools\idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Launcher build failed!
    pause
    exit /b 1
)
python %IDF_PATH%\tools\idf.py -p COM8 flash
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Launcher flash failed!
    pause
    exit /b 1
)

echo.
echo [2/4] Building PAC-MAN (OTA_0 partition)...
cd /d "%~dp0Apps\pacman"
python %IDF_PATH%\tools\idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Pac-Man build failed!
    pause
    exit /b 1
)

echo.
echo [3/4] Building TETRIS (OTA_1 partition)...
cd /d "%~dp0Apps\tetris"
python %IDF_PATH%\tools\idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Tetris build failed!
    pause
    exit /b 1
)

echo.
echo [4/4] Building FROGGER (OTA_2 partition)...
cd /d "%~dp0Apps\frogger"
python %IDF_PATH%\tools\idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Frogger build failed!
    pause
    exit /b 1
)

echo.
echo All games built successfully!
echo.
echo Flashing PAC-MAN to OTA_0...
python -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x110000 "%~dp0Apps\pacman\build\pacman.bin"

echo.
echo Flashing TETRIS to OTA_1...
python -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x200000 "%~dp0Apps\tetris\build\tetris.bin"

echo.
echo Flashing FROGGER to OTA_2...
python -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x2F0000 "%~dp0Apps\frogger\build\frogger.bin"

echo.
echo ========================================
echo All apps flashed successfully!
echo ========================================
echo.
echo Press RESET on your eBadge to start the Game Launcher.
echo Use LEFT/RIGHT to select a game, press A to launch it.
echo.
pause
