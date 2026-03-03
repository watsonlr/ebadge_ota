@echo off
REM Flash Frogger to OTA_2 partition
set PYTHON=C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe

echo Flashing FROGGER to OTA_2 partition at 0x2F0000...
"%PYTHON%" -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0x2F0000 Apps\frogger\build\frogger_game.bin
if errorlevel 1 (
    echo FROGGER flash failed!
    pause
    exit /b 1
)

echo.
echo FROGGER flashed successfully!
echo.
