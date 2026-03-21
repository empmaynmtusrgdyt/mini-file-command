@echo off
cd /d "%~dp0\.."
if not exist data\magic.db python scripts\parser.py
gcc src\main.c src\sqlite3.c -o -Isrc mini-file-command.exe
echo Сборка завершена. Запускайте программу командой: mini-file-command.exe [FILE]
pause