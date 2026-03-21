#!/bin/bash
gcc src/main.c src/sqlite3.c -lpthread -ldl -Isrc -o mini-file-command
echo "Сборка завершена. Запускайте программу командой: ./mini-file-command [FILE]"