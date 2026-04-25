# mini-file-command

Утилита командной строки для определения типа файла по сигнатурам.

## Описание

`mini-file-command` анализирует файл и определяет его тип на основе базы сигнатур. Утилита читает первые байты файла и ищет совпадение в базе данных, возвращая описание формата и предполагаемое расширение. Данная программа является кроссплатформенной.

## Требования

- GCC (или другой C-компилятор)


## Сборка и запуск

### Linux/macOS

```bash
./scripts/build-and-run-unix.sh
./mini-file-command [ФАЙЛ]
```

### Windows

```cmd
scripts\build-and-run-win.bat
mini-file-command.exe [ФАЙЛ]
```

## Структура проекта

```
.
├── data/
│   └── file_sigs.json          # база сигнатур
├── scripts/
│   ├── parser.c                # скрипт создания БД из JSON
│   ├── build-and-run-unix.sh   # скрипт для запуска под Linux/macOS
│   └── build-and-run-win.bat   # скрипт для запуска под Windows
├── src/
│   ├── main.c                  # основной исходный код
│   ├── sqlite3.c               # исходный код SQLite
│   ├── sqlite3.h               # заголовочный файл SQLite
│   ├── cJSON.c                 # исходный код cJSON
│   └── cJSON.h                 # заголовочный файл cJSON
├── AUTHORS
├── CONTRIBUTING
├── LICENSE
└── README.md
```

## Используемые компоненты

- `src/sqlite3.c`, `src/sqlite3.h` — [SQLite amalgamation](https://www.sqlite.org/amalgamation.html)
- `data/file_sigs.json` — [GCK's File Signature Table](https://github.com/toreaurstadboss/FileHeaderUtil/blob/main/file_sigs.json)
- `src/cJSON.c`, `src/cJSON.h` - [cJSON](https://github.com/DaveGamble/cJSON)

## Лицензия

GNU General Public License v3.0 или новее. См. файл [LICENSE](LICENSE).

## Автор

Marth Scoobert <leammax@yandex.ru>