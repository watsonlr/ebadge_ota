@echo off
cd /d C:\Users\lynn\Documents\Repositories\ebadge_ota\Apps\game_launcher
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.3.1
set PATH=C:\Espressif\tools\cmake\3.24.0\bin;C:\Espressif\tools\ninja\1.11.1;C:\Espressif\tools\xtensa-esp-elf\esp-13.2.0_20240530\xtensa-esp-elf\bin;C:\Espressif\tools\riscv32-esp-elf\esp-13.2.0_20240530\riscv32-esp-elf\bin;C:\Espressif\python_env\idf5.3_py3.11_env\Scripts;%PATH%
python %IDF_PATH%\tools\idf.py -p COM8 flash
