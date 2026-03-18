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
    if(result_2 != 16){
        printf("ERROR: Ошибка чтения данных из файла");
        exit_code = 5;
        goto close_bd_and_file;
    }

    for(size_t i = 0; i < result_2; i++){
        printf("%X", buffer[i]);
    }

    close_bd_and_file:
        result = fclose(f);
        if(result != 0){
            printf("ERROR: Ошибка закрытия файла");
            exit_code = 7;
        }
        result = sqlite3_close(db);
        if(result != SQLITE_OK){
            printf("ERROR: Ошибка закрытия базы данных");
            exit_code = 6;
        }
        return exit_code;
}

// TODO: добавить парсинг аргументов командой строки + флаги (--version --help)
// Ладно, окей, это оказалось сложнее чем я думал
// Для начала сделаю так чтобы путь к файлу и флаги просто вводились юзером через scanf()
// Парсинг аргов возможно сделаю потом
// Сейчас сосредоточусь на чтениии байтов из какого-нить файла