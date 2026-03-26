# mini-file-command

Утилита командной строки для определения типа файла по сигнатурам.

## Описание

`mini-file-command` анализирует заголовок файла и определяет его тип на основе базы сигнатур. Утилита читает первые байты файла и ищет совпадение в базе данных, возвращая описание формата и предполагаемое расширение. Данная программа является кроссплатформенной: работает на Windows, Linux и macOS.

## Требования

- GCC (или другой C-компилятор)
- Python 3 (для создания базы данных)

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
├── src/
│   ├── main.c          # основной исходный код
│   ├── sqlite3.c       # исходный код SQLite
│   └── sqlite3.h       # заголовочный файл SQLite
├── data/
│   └── file_sigs.json  # база сигнатур
├── scripts/
│   ├── parser.py             # скрипт создания БД из JSON
│   ├── build-and-run-unix.sh # скрипт для запуска под Linux/macOS
│   └── build-and-run-win.bat # скрипт для запуска под Windows
├── AUTHORS
├── CONTRIBUTING
├── LICENSE
└── README.md
```

## Используемые компоненты

- `src/sqlite3.c`, `src/sqlite3.h` — [SQLite amalgamation](https://www.sqlite.org/amalgamation.html)
- `data/file_sigs.json` — [GCK's File Signature Table](https://github.com/toreaurstadboss/FileHeaderUtil/blob/main/file_sigs.json)

## Лицензия

GNU General Public License v3.0 или новее. См. файл [LICENSE](LICENSE).

## Автор

Marth Scoobert <leammax@yandex.ru>