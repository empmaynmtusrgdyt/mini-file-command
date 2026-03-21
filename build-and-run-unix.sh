#!/bin/bash
gcc main.c sqlite3.c -lpthread -ldl -o mini-file-command
./mini-file-command