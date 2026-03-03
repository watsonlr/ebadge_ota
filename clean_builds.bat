@echo off
REM Clean all build directories and rebuild from scratch
echo Cleaning all build directories...
if exist Apps\pacman\build rmdir /s /q Apps\pacman\build
if exist Apps\tetris\build rmdir /s /q Apps\tetris\build
if exist Apps\frogger\build rmdir /s /q Apps\frogger\build

echo.
echo Build directories cleaned.
echo Please open a native Windows Command Prompt (not WSL)
echo Navigate to: C:\Users\lynn\Documents\Repositories\ebadge_ota
echo Then run: flash_games_only.bat
echo.
pause
