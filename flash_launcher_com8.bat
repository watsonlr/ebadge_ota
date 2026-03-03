@echo off
REM Quick Flash Game Launcher to COM8
REM Run this from ESP-IDF PowerShell or ESP-IDF CMD

cd /d "%~dp0\Apps\game_launcher"

where idf.py >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Run this from ESP-IDF PowerShell or ESP-IDF CMD
    pause
    exit /b 1
)

echo Building and flashing Game Launcher to COM8...
echo.

idf.py build
if %ERRORLEVEL% NEQ 0 exit /b 1

echo.
echo Flashing to COM8...
echo.
idf.py -p COM8 flash monitor
