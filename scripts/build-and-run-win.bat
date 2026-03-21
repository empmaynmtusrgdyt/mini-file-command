@echo off
gcc src/main.c src/sqlite3.c -o mini-file-command.exe
echo Сборка завершена. Запускайте программу командой: mini-file-command.exe [FILE]