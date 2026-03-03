@echo off
REM Simple build script - builds each app one at a time
REM Run this from ESP-IDF PowerShell or ESP-IDF CMD

echo ============================================
echo Building eBadge Apps
echo ============================================
echo.
echo This will take approximately 10-15 minutes...
echo.

cd /d "%~dp0"

REM Check if idf.py is available
where idf.py >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: idf.py not found!
    echo.
    echo Please run this script from ESP-IDF PowerShell or ESP-IDF CMD
    echo You can find it in the Start Menu after installing ESP-IDF
    echo.
    pause
    exit /b 1
)

echo Found idf.py, proceeding with build...
echo.

REM Pac-Man
echo ============================================
echo [1/4] Building Pac-Man Game
echo ============================================
cd Apps\pacman
idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Pac-Man build failed!
    echo Check the error messages above.
    pause
    exit /b 1
)
echo.
echo SUCCESS: Pac-Man built!
echo.

REM Tetris
echo ============================================
echo [2/4] Building Tetris Game
echo ============================================
cd ..\tetris
idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Tetris build failed!
    pause
    exit /b 1
)
echo.
echo SUCCESS: Tetris built!
echo.

REM Frogger
echo ============================================
echo [3/4] Building Frogger Game
echo ============================================
cd ..\frogger
idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Frogger build failed!
    pause
    exit /b 1
)
echo.
echo SUCCESS: Frogger built!
echo.

REM Game Launcher
echo ============================================
echo [4/4] Building Game Launcher
echo ============================================
cd ..\game_launcher
idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Game Launcher build failed!
    pause
    exit /b 1
)
echo.
echo SUCCESS: Game Launcher built!
echo.

REM Return to root
cd ..\..

REM Create OTA directory if needed
if not exist "ota_files\apps" mkdir ota_files\apps

REM Copy binaries
echo ============================================
echo Copying binaries to OTA directory...
echo ============================================
echo.

copy /Y "Apps\pacman\build\pacman_game.bin" "ota_files\apps\pacman.bin" >nul
if exist "ota_files\apps\pacman.bin" echo [OK] pacman.bin

copy /Y "Apps\tetris\build\tetris_game.bin" "ota_files\apps\tetris.bin" >nul
if exist "ota_files\apps\tetris.bin" echo [OK] tetris.bin

copy /Y "Apps\frogger\build\frogger_game.bin" "ota_files\apps\frogger.bin" >nul
if exist "ota_files\apps\frogger.bin" echo [OK] frogger.bin

copy /Y "Apps\game_launcher\build\game_launcher.bin" "ota_files\apps\launcher.bin" >nul
if exist "ota_files\apps\launcher.bin" echo [OK] launcher.bin

echo.
echo ============================================
echo BUILD COMPLETE!
echo ============================================
echo.
echo All 4 apps built successfully:
echo   1. Pac-Man      - Apps\pacman\build\pacman_game.bin
echo   2. Tetris       - Apps\tetris\build\tetris_game.bin
echo   3. Frogger      - Apps\frogger\build\frogger_game.bin
echo   4. Game Launcher- Apps\game_launcher\build\game_launcher.bin
echo.
echo Binaries copied to: ota_files\apps\
echo.
echo Next Steps:
echo   1. Flash the Game Launcher:
echo      cd Apps\game_launcher
echo      idf.py -p COM3 flash
echo.
echo   2. Or flash individual games:
echo      cd Apps\pacman
echo      idf.py -p COM3 flash
echo.
echo   3. Start OTA server:
echo      python simple_ota_server.py
echo.
pause
