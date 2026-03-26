#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#ifdef _WIN32
    #include <Windows.h>
    #define MAX_PATH_LEN MAX_PATH
#else
    #include <limits.h>
    #define MAX_PATH_LEN PATH_MAX
#endif

#define PROGRAM_NAME "mini-file-command"
#define PROGRAM_VERSION "2.0.0"


int main(int argc, char *argv[]){
    int exit_code = 0;
    sqlite3 *db = NULL;
    int result = 0;
    size_t result_2 = 0;
    char path[MAX_PATH_LEN] = {0};
    FILE * f = NULL;
    sqlite3_stmt *stmt = NULL;
    unsigned char first_bytes[8192];
    int flag = 0;

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

    strncpy(path, argv[1], MAX_PATH_LEN - 1);
    path[MAX_PATH_LEN - 1] = '\0';

    result = sqlite3_open("data/magic.db", &db);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка открытия базы данных");
        return 2;
    }

    f = fopen(path, "rb");
    if(f == NULL){
        printf("ERROR: Ошибка открытия файла");
        exit_code = 3;
        goto close_bd_and_file;
    }

    for(size_t i = 0; i < (sizeof(first_bytes) / sizeof(first_bytes[0])); i++){
        result = fgetc(f);
        if(result == EOF){
            printf("ERROR: Ошибка чтения данных из файла");
            exit_code = 4;
            goto close_bd_and_file;
        }
        first_bytes[i] = result;
    }

    char *sql = "SELECT hex_sig, sig_length, offset, trailer, extension, description FROM signatures ORDER BY sig_length DESC";
    result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка работы базы данных");
        exit_code = 5;
        goto close_bd_and_file;
    }

    while(sqlite3_step(stmt) == SQLITE_ROW){
        const void * hex_sig = sqlite3_column_blob(stmt, 0);
        int sig_length = sqlite3_column_int(stmt, 1);
        int offset = sqlite3_column_int(stmt, 2);
        const void * trailer = sqlite3_column_blob(stmt, 3);
        int trailer_length = sqlite3_column_bytes(stmt, 3);
        const unsigned char * ext = sqlite3_column_text(stmt, 4);
        const unsigned char * desc = sqlite3_column_text(stmt, 5);

        if(offset + sig_length > (sizeof(first_bytes) / sizeof(first_bytes[0]))) continue;
        unsigned char offset_bytes[sig_length];
        memcpy(offset_bytes, &first_bytes[offset], sig_length);
        size_t flag_for_sig_length = 0;

        for(size_t i = 0; i < sig_length; i++){
            if(((unsigned char *)hex_sig)[i] != offset_bytes[i]){
                break;
            }
            else flag_for_sig_length++;
        }

        if(flag_for_sig_length != sig_length) continue;
        if(trailer != NULL && trailer_length > 0){
            long current_position = ftell(f);
            if(current_position == -1){
                printf("ERROR: Ошибка при работе с трейлером");
                continue;
            }
            result = fseek(f, -trailer_length, SEEK_END);
            if(result != 0){
                printf("ERROR: Ошибка при работе с трейлером");
                result = fseek(f, current_position, SEEK_SET);
                if(result != 0){
                    printf("ERROR: Ошибка при работе с трейлером");
                    continue;
                }
                continue;
            }
            unsigned char file_trailer[trailer_length];
            result_2 = fread(file_trailer, sizeof(unsigned char), trailer_length, f);
            if(result_2 != trailer_length){
                printf("ERROR: Ошибка при работе с трейлером");
                result = fseek(f, current_position, SEEK_SET);
                if(result != 0){
                    printf("ERROR: Ошибка при работе с трейлером");
                    continue;
                }
                continue;
            }
            result = fseek(f, current_position, SEEK_SET);
            if(result != 0){
                printf("ERROR: Ошибка при работе с трейлером");
                continue;
            }
            if(memcmp(trailer, file_trailer, trailer_length) != 0){
                result = fseek(f, current_position, SEEK_SET);
                if(result != 0){
                    printf("ERROR: Ошибка при работе с трейлером");
                    continue;
                }
                continue;
            }
            result = fseek(f, current_position, SEEK_SET);
            if(result != 0){
                continue;
            }

        }
        flag = 1;
        printf("Описание: %s\n", desc);
        printf("Расширение: %s\n", ext);
        break;
    }

    if(flag == 0){
        printf("Тип файла не определен. Нет совпадений в базе\n");
        exit_code = 6;
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

// Остался только README



// Первая версия программы была готова, но она не идеальна
// Основная проблема кроется именно в json-файле который я использовал до этого
// Сильно углубляться не буду, ибо это не так важно. Так или иначе теперь я буду использовать новый json
// Хотя, нет, ладно, наверное про проблему все-таки стоит рассказать
// Итак, суть в том, что в старом json-файле (автором которого между прочим явялется какой-то индус с почти 10к фоловерами на гитхабе)
// Так вот, в старом json-файле была существенная проблема в том, что первые байты совпадали для очень большого кол-ва форматов
// В новом же файле эта проблема решается за счет смещения и более длинных/точных сигнатур 
// Конечно, парсить будет гораздо сложнее (питон-файл придется переписать практически полностью)
// Но я надеюсь что это принесет свои плоды и работа программы станет лучше
// Короче погнали

// Ну вроде парсер работает, бд создается

// Теперь надо идти дальше: просмотреть весь код и максимально обновить его 
// Чтобы он тянул правильные данные из базы (это выглядит несложно)

// Ладно, надо жестко комментарии пописать, а то что-то я туплю

// TODO: проверить выполнены ли все проверки; дописать комменты; поправить статус-коды

// Ну я вроде дописал. Теперь надо удалить ненужные куски, убрать все лишнее и протестить 