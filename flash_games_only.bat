@echo off
REM This script builds and flashes only the 3 games (assumes launcher is already flashed)
echo ========================================
echo Building and Flashing 3 Games Only
echo ========================================

set PATH=C:\Espressif\tools\cmake\3.24.0\bin;C:\Espressif\tools\ninja\1.11.1;C:\Espressif\tools\xtensa-esp-elf\esp-13.2.0_20240530\xtensa-esp-elf\bin;%PATH%
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1
set PYTHON=C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe

echo.
echo [1/6] Building PAC-MAN...
cd Apps\pacman
"%PYTHON%" "%IDF_PATH%\tools\idf.py" build
if errorlevel 1 (
    echo PAC-MAN build failed!
    cd ..\..
    pause
    exit /b 1
)
cd ..\..

echo.
echo [2/6] Flashing PAC-MAN to OTA_0 partition at 0x110000...
"%PYTHON%" -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x110000 Apps\pacman\build\pacman_game.bin
if errorlevel 1 (
    echo PAC-MAN flash failed!
    pause
    exit /b 1
)

echo.
echo [3/6] Building TETRIS...
cd Apps\tetris
"%PYTHON%" "%IDF_PATH%\tools\idf.py" build
if errorlevel 1 (
    echo TETRIS build failed!
    cd ..\..
    pause
    exit /b 1
)
cd ..\..

echo.
echo [4/6] Flashing TETRIS to OTA_1 partition at 0x200000...
"%PYTHON%" -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x200000 Apps\tetris\build\tetris_game.bin
if errorlevel 1 (
    echo TETRIS flash failed!
    pause
    exit /b 1
)

echo.
echo [5/6] Building FROGGER...
cd Apps\frogger
"%PYTHON%" "%IDF_PATH%\tools\idf.py" build
if errorlevel 1 (
    echo FROGGER build failed!
    cd ..\..
    pause
    exit /b 1
)
cd ..\..

echo.
echo [6/6] Flashing FROGGER to OTA_2 partition at 0x2F0000...
"%PYTHON%" -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x2F0000 Apps\frogger\build\frogger_game.bin
if errorlevel 1 (
    echo FROGGER flash failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo All games built and flashed successfully!
echo Press RESET on your device to play!
echo ========================================
pause
