@echo off
cd /d "%~dp0\.."
gcc scripts\parser.c src\cJSON.c src\sqlite3.c -Isrc -o scripts\parser_c.exe
scripts\parser_c.exe
gcc src\main.c src\sqlite3.c src\cJSON.c -o -Isrc mini-file-command.exe
echo Сборка завершена. Запускайте программу командой: mini-file-command.exe [FILE]
pause