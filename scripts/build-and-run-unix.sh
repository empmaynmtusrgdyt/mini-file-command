#!/bin/bash
cd "$(dirname "$0")/.."
gcc scripts/parser.c src/cJSON.c src/sqlite3.c -lpthread -ldl -Isrc -o scripts/parser_c
./scripts/parser_c
gcc src/main.c src/sqlite3.c src/cJSON.c -lpthread -ldl -Isrc -o mini-file-command
echo "Сборка завершена. Запускайте программу командой: ./mini-file-command [FILE]"