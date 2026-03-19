#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int main(){
    int exit_code = 0;
    sqlite3 *db;
    int result;
    size_t result_2;
    char path[1000];
    FILE * f;
    unsigned char buffer[16];
    sqlite3_stmt *stmt;

    result = sqlite3_open("magic.db", &db);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка открытия базы данных");
        return 1;
    }
    
    result = printf("Введите путь к файлу: ");
    if(result < 0){
        printf("ERROR: Ошибка вывода");
        exit_code = 2;
        goto close_bd_and_file;
    }

    result = scanf("%999s", path);
    if(result != 1){
        printf("ERROR: Ошибка ввода");
        exit_code = 3;
        goto close_bd_and_file;
    }

    f = fopen(path, "rb");
    if(f == NULL){
        printf("ERROR: Ошибка открытия файла");
        exit_code = 4;
        goto close_bd_and_file;
    }

    result_2 = fread(buffer, sizeof(char), 16, f);
    if(result_2 <= 0){
        printf("ERROR: Ошибка чтения данных из файла");
        exit_code = 5;
        goto close_bd_and_file;
    }

    /*for(size_t i = 0; i < result_2; i++){
        printf("%X", buffer[i]);
    }*/

    char *sql = "SELECT description, extension FROM signatures WHERE hex_sig = substr(?, 1, length(hex_sig)) LIMIT 1";
    result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка работы базы данных");
        exit_code = 6;
        goto close_bd_and_file;
    }

    result = sqlite3_bind_blob(stmt, 1, buffer, 16, SQLITE_STATIC);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка привзяки BLOB");
        exit_code = 7;
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
        exit_code = 8;
        goto close_bd_and_file;
    }

    close_bd_and_file:
        if(f != NULL){
            result = fclose(f);
            if(result != 0){
                printf("ERROR: Ошибка закрытия файла");
                exit_code = 9;
            }
        }
        result = sqlite3_finalize(stmt);
        if(result != SQLITE_OK){
            printf("ERROR: Ошибка очистки");
            exit_code = 10;
        }
        result = sqlite3_close(db);
        if(result != SQLITE_OK){
            printf("ERROR: Ошибка закрытия базы данных");
            exit_code = 11;
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