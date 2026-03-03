@echo off
REM Monitor serial output from COM8
set PYTHON=C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1

echo Starting serial monitor on COM8...
echo Press Ctrl+] to exit
echo.

"%PYTHON%" -m serial.tools.miniterm --eol LF --raw COM8 115200
