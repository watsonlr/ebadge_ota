@echo off
REM Build and Flash Game Launcher
REM Run this from ESP-IDF PowerShell or ESP-IDF CMD

echo ============================================
echo Build and Flash Game Launcher
echo ============================================
echo.

cd /d "%~dp0\Apps\game_launcher"

REM Check if idf.py is available
where idf.py >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: idf.py not found!
    echo.
    echo Please run this script from ESP-IDF PowerShell or ESP-IDF CMD
    echo.
    pause
    exit /b 1
)

echo Step 1: Building Game Launcher...
echo.
idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo Build successful!
echo ============================================
echo.
echo Binary: Apps\game_launcher\build\game_launcher.bin
echo.
echo.
echo Step 2: Flashing to device...
echo.
echo Please connect your eBadge via USB
echo.

REM Try to detect COM port
set DETECTED_PORT=
for /f "tokens=1" %%A in ('mode ^| findstr /C:"COM"') do set DETECTED_PORT=%%A

if defined DETECTED_PORT (
    echo Detected port: %DETECTED_PORT%
    echo.
    set /p USE_PORT="Use this port? (Y/n): "
    if /i "!USE_PORT!"=="n" (
        set /p FLASH_PORT="Enter COM port (e.g., COM3): "
    ) else (
        set FLASH_PORT=%DETECTED_PORT:~0,-1%
    )
) else (
    echo.
    set /p FLASH_PORT="Enter COM port (e.g., COM3): "
)

echo.
echo Flashing to %FLASH_PORT%...
echo.
idf.py -p %FLASH_PORT% flash

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo Flash Complete!
    echo ============================================
    echo.
    echo The Game Launcher is now running on your eBadge
    echo Press RESET button to start the menu
    echo.
    echo Controls:
    echo   UP/DOWN - Navigate menu
    echo   A Button - Launch selected game
    echo.
    set /p MONITOR="Open serial monitor? (Y/n): "
    if /i "!MONITOR!"=="n" (
        goto :end
    ) else (
        echo.
        echo Opening monitor... Press CTRL+] to exit
        echo.
        idf.py -p %FLASH_PORT% monitor
    )
) else (
    echo.
    echo ERROR: Flash failed!
    echo.
    echo Troubleshooting:
    echo   1. Check USB cable is connected
    echo   2. Verify correct COM port in Device Manager
    echo   3. Close other serial programs
    echo   4. Try holding BOOT button while flashing
    echo.
)

:end
pause
