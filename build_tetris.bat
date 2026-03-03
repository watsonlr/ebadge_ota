@echo off
echo Building TETRIS...
set PATH=C:\Espressif\tools\cmake\3.24.0\bin;C:\Espressif\tools\ninja\1.11.1;C:\Espressif\tools\xtensa-esp-elf\esp-13.2.0_20240530\xtensa-esp-elf\bin;%PATH%
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1
cd Apps\tetris
"C:\Espressif\python_env\idf5.3_py3.11_env\Scripts\python.exe" "%IDF_PATH%\tools\idf.py" build
cd ..\..
