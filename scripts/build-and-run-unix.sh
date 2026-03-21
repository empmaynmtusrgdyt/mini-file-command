#!/bin/bash
cd "$(dirname "$0")/.."
[ ! -f data/magic.db ] && python3 scripts/parser.py
gcc src/main.c src/sqlite3.c -lpthread -ldl -Isrc -o mini-file-command
echo "Сборка завершена. Запускайте программу командой: ./mini-file-command [FILE]"