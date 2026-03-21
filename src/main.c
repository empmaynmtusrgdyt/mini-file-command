#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define MAX 1000
#define PROGRAM_NAME "mini-file-command"
#define PROGRAM_VERSION "1.0.0"

int main(int argc, char *argv[]){
    int exit_code = 0;
    sqlite3 *db = NULL;
    int result = 0;
    size_t result_2 = 0;
    char path[MAX] = {0};
    FILE * f = NULL;
    unsigned char buffer[16] = {0};
    sqlite3_stmt *stmt = NULL;

    if(argc != 2){
        printf("ERROR: Ошибка количества аргументов\n");
        printf("Введите ./mini-file-command --help для справки\n");
        return 1;
    }

    if((strcmp(argv[1], "--version")) == 0 || (strcmp(argv[1], "-v")) == 0){
        printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
        printf("Copyright (C) 2025 Free Software Foundation, Inc.\n");
        printf("License GPLv3+: GNU GPL version 3 or later\n");
        printf("This is free software: you are free to change and redistribute it.\n");
        printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
        printf("Written by Marth Scoobert\n");
        return 0;
    }

    if((strcmp(argv[1], "--help")) == 0 || (strcmp(argv[1], "-h")) == 0){
        printf("Usage: %s [OPTION] [FILE]\n", PROGRAM_NAME);
        printf("Determine file type based on magic numbers from the database.\n\n");
        printf("Options:\n");
        printf("  -h, --help                display this help and exit\n");
        printf("  -v, --version             output version information and exit\n\n");
        printf("Arguments:\n");
        printf("  FILE                      path to the file to be identified.\n");
        printf("                            Use quotes if the path contains spaces.\n\n");
        printf("Examples:\n");
        printf("  %s image.jpg\n", PROGRAM_NAME);
        printf("  %s \"C:\\My Files\\data.bin\"\n\n", PROGRAM_NAME);
        printf("Report bugs to leammax@yandex.ru\n");
        return 0;
    }

    strncpy(path, argv[1], MAX - 1);

    result = sqlite3_open("data/magic.db", &db);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка открытия базы данных");
        return 2;
    }
    
    /*result = printf("Введите путь к файлу: ");
    if(result < 0){
        printf("ERROR: Ошибка вывода");
        exit_code = 3;
        goto close_bd_and_file;
    }

    if(fgets(path, MAX, stdin) == NULL){
        printf("ERROR: Ошибка ввода");
        exit_code = 4;
        goto close_bd_and_file;
    }

    size_t len = strlen(path);
    if(len > 0 && path[len - 1] == '\n'){
        path[len - 1] = '\0';
    }*/

    f = fopen(path, "rb");
    if(f == NULL){
        printf("ERROR: Ошибка открытия файла");
        exit_code = 3;
        goto close_bd_and_file;
    }

    result_2 = fread(buffer, sizeof(char), 16, f);
    if(result_2 <= 0){
        printf("ERROR: Ошибка чтения данных из файла");
        exit_code = 4;
        goto close_bd_and_file;
    }

    /*for(size_t i = 0; i < result_2; i++){
        printf("%X", buffer[i]);
    }*/

    char *sql = "SELECT description, extension FROM signatures WHERE hex_sig = substr(?, 1, length(hex_sig)) LIMIT 1";
    result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка работы базы данных");
        exit_code = 5;
        goto close_bd_and_file;
    }

    result = sqlite3_bind_blob(stmt, 1, buffer, 16, SQLITE_STATIC);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка привзяки BLOB");
        exit_code = 6;
        goto close_bd_and_file;
    }

    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW){
        const unsigned char *desc = sqlite3_column_text(stmt, 0);
        const unsigned char *ext = sqlite3_column_text(stmt, 1);

        printf("Тип файла определен\n");
        printf("Описание: %s\n", desc);
        printf("Расширение: %s\n", ext);
    }
    else if(result == SQLITE_DONE){
        printf("Тип файла не определен. Нет совпадений в базе\n");
    }
    else{
        printf("ERROR: Ошибка поиска в базе данных");
        exit_code = 7;
        goto close_bd_and_file;
    }

    close_bd_and_file:
        if(f != NULL){
            result = fclose(f);
            if(result != 0){
                printf("ERROR: Ошибка закрытия файла");
                exit_code = 8;
            }
        }
        result = sqlite3_finalize(stmt);
        if(result != SQLITE_OK){
            printf("ERROR: Ошибка очистки");
            exit_code = 9;
        }
        result = sqlite3_close(db);
        if(result != SQLITE_OK){
            printf("ERROR: Ошибка закрытия базы данных");
            exit_code = 10;
        }
        return exit_code;
}

// TODO: добавить парсинг аргументов командой строки + флаги (--version --help)
// Ладно, окей, это оказалось сложнее чем я думал
// Для начала сделаю так чтобы путь к файлу и флаги просто вводились юзером через scanf()
// Парсинг аргов возможно сделаю потом
// Сейчас сосредоточусь на чтениии байтов из какого-нить файла

// Так, ладно, это готово
// Теперь самое время добавить основную логику: 
// Создание sql-запроса к базе и более-менее нормальный вывод в консоль

// Надо протестить

// Ну вроде работает

// Ну вроде все, тестим