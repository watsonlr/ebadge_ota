@echo off
REM Reset OTA data to boot to launcher (factory partition)
set PYTHON=C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe

echo Resetting OTA data to boot to launcher...
"%PYTHON%" -m esptool --chip esp32s3 -p COM8 -b 460800 write_flash 0xf000 Apps\game_launcher\build\ota_data_initial.bin
if errorlevel 1 (
    echo OTA data reset failed!
    pause
    exit /b 1
)

echo.
echo OTA data reset! Device will now boot to launcher.
echo.
