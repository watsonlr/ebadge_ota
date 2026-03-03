@echo off
REM Build all games and launcher for ESP32-S3 eBadge
REM Run this from ESP-IDF PowerShell or CMD environment

echo ============================================
echo Building All eBadge Apps
echo ============================================
echo.

REM Build Pac-Man
echo [1/4] Building Pac-Man...
cd Apps\pacman
call idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Pac-Man build failed!
    exit /b 1
)
echo Pac-Man build successful!
echo.

REM Build Tetris
echo [2/4] Building Tetris...
cd ..\tetris
call idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Tetris build failed!
    exit /b 1
)
echo Tetris build successful!
echo.

REM Build Frogger
echo [3/4] Building Frogger...
cd ..\frogger
call idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Frogger build failed!
    exit /b 1
)
echo Frogger build successful!
echo.

REM Build Game Launcher
echo [4/4] Building Game Launcher...
cd ..\game_launcher
call idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Game Launcher build failed!
    exit /b 1
)
echo Game Launcher build successful!
echo.

REM Copy binaries to OTA directory
echo ============================================
echo Copying binaries to OTA directory...
echo ============================================

cd ..\..

if not exist "ota_files\apps" mkdir ota_files\apps

copy /Y "Apps\pacman\build\pacman_game.bin" "ota_files\apps\pacman.bin"
copy /Y "Apps\tetris\build\tetris_game.bin" "ota_files\apps\tetris.bin"
copy /Y "Apps\frogger\build\frogger_game.bin" "ota_files\apps\frogger.bin"
copy /Y "Apps\game_launcher\build\game_launcher.bin" "ota_files\apps\launcher.bin"

echo.
echo ============================================
echo Build Complete!
echo ============================================
echo.
echo Binaries copied to ota_files\apps\:
echo   - pacman.bin
echo   - tetris.bin
echo   - frogger.bin
echo   - launcher.bin
echo.
echo To flash the game launcher:
echo   cd Apps\game_launcher
echo   idf.py -p COMX flash
echo.
echo To start OTA server:
echo   python simple_ota_server.py
echo.

pause
