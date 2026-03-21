@echo off
gcc main.c sqlite3.c -o mini-file-command.exe
mini-file-command.exe %*
pause