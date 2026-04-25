#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <cJSON.h>


int hex_to_bytes(char* original_string, unsigned char* hex_string, size_t* hex_string_len){
    if(!original_string) return -1;
    if(strcmp(original_string, "(null)") == 0) return -1;
    size_t j = 0;
    size_t k = 0;
    size_t i = 0;
    size_t c = 0;
    size_t counter = 0;
    size_t original_string_without_spaces_len = 0;
    size_t original_string_strlen = strlen(original_string);

    while(j < original_string_strlen){
        if(original_string[j] == ' '){
            j++;
            continue;
        }
        original_string_without_spaces_len++;
        j++;
    }
    if(original_string_without_spaces_len % 2 != 0) return -1;

    while(k < original_string_strlen){
        if(!isxdigit(original_string[k]) && original_string[k] != ' '){
            return -1;
        }
        k++;
    }

    while(i < original_string_strlen){
        if(original_string[i] == ' '){
            i++;
            continue;
        }
        char buffer[3] = "";
        buffer[0] = original_string[i];
        i++;
        while(original_string[i] == ' ') i++;
        if(original_string[i] == '\0') return -1;
        buffer[1] = original_string[i];
        buffer[2] = '\0';
        unsigned char val = strtol(buffer, NULL, 16);
        hex_string[c] = val;
        c++;
        i++;
    }
    *hex_string_len = c;
    return 0;
}


long parse_offset(char* offset_str){
    if(!offset_str) return -1;
    size_t offset_str_len = strlen(offset_str);
    size_t i = 0;
    for(i = 0; i < offset_str_len; i++){
        if(isdigit(offset_str[i])){
            break;
        }
    }
    if(i == offset_str_len) return 0;
    long val = strtol(&offset_str[i], NULL, 10);
    return val;
}


long get_file_size(char* filename){
    FILE * f;
    int check_fseek = 0;
    int chech_fclose = 0;
    long file_size;
    f = fopen(filename, "rb");
    if(f == NULL){
        printf("ERROR: Ошибка внутри функции в открытие файла");
        return 1;
    }
    check_fseek = fseek(f, 0, SEEK_END);
    if(check_fseek != 0){
        printf("ERROR: Ошибка внутри функции в переходе в конец файла");
        return 2;
    }
    file_size = ftell(f);
    if(file_size == -1){
        printf("ERROR: Ошибка внутри функции в определении размера файла");
        return 3;
    }
    chech_fclose = fclose(f);
    if(chech_fclose != 0){
        printf("ERROR: Ошибка внутри функции в закрытии файла");
        return 4;
    }
    return file_size;
}


int main(){
    sqlite3 *db = NULL;
    int result = 0;
    size_t result_st = 0;
    long json_file_size = 0;
    char *json_file_content = NULL;
    FILE * f = NULL;
    cJSON *json_var = NULL;
    cJSON *filesigs_array_from_json_file = NULL;
    cJSON *one_filesig_from_array = NULL;
    sqlite3_stmt* stmt = NULL;

    result = sqlite3_open("data/magic.db", &db);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка открытия базы данных");
        sqlite3_close(db);
        return 1;
    }
    char *sql = "DROP TABLE IF EXISTS signatures; CREATE TABLE signatures(id INTEGER PRIMARY KEY, extension TEXT, description TEXT, hex_sig BLOB, sig_length INTEGER, offset INTEGER DEFAULT 0, trailer BLOB, trailer_len INTEGER); CREATE UNIQUE INDEX IF NOT EXISTS idx_sig_full ON signatures (hex_sig, sig_length, offset, trailer, trailer_len, description, extension);";
    result = sqlite3_exec(db, sql, 0, 0, 0);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка создания таблицы");
        sqlite3_close(db);
        return 2;
    }

    json_file_size = get_file_size("data/file_sigs.json");
    json_file_content = malloc((json_file_size + 1) * sizeof(json_file_content[0])); // +1 для терминального нуля
    if(json_file_content == NULL){
        printf("ERROR: Ошибка выделения памяти для массива (всего содержимого json-файла)");
        sqlite3_close(db);
        return 3;
    }

    f = fopen("data/file_sigs.json", "rb");
    if(f == NULL){
        printf("ERROR: Ошибка в открытии json-файла");
        sqlite3_close(db);
        free(json_file_content);
        return 4;
    }
    result_st = fread(json_file_content, sizeof(char), json_file_size, f);
    if(result_st != json_file_size){
        printf("ERROR: Ошибка в чтении данных из json-файла в массив");
        sqlite3_close(db);
        free(json_file_content);
        return 5;
    }
    json_file_content[json_file_size] = '\0';
    result = fclose(f);
    if(result != 0){
        printf("ERROR: Ошибка в закрытии json-файла");
        sqlite3_close(db);
        free(json_file_content);
        return 6;
    }

    // на этом этапе я имею массив json_file_content, в котором записано все содержимое json-файла

    json_var = cJSON_Parse(json_file_content);
    if(json_var == NULL){
        printf("ERROR: Ошибка в создании обьекта json_var");
        sqlite3_close(db);
        free(json_file_content);
        return 7;
    }

    filesigs_array_from_json_file = cJSON_GetObjectItemCaseSensitive(json_var, "filesigs");
    if(!cJSON_IsArray(filesigs_array_from_json_file)){
        printf("ERROR: Ошибка в несоответствии типов данных");
        sqlite3_close(db);
        free(json_file_content);
        return 8;
    }

    sql = "INSERT OR IGNORE INTO signatures (extension, description, hex_sig, sig_length, offset, trailer, trailer_len) VALUES (?, ?, ?, ?, ?, ?, ?)";
    result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if(result != SQLITE_OK){
        printf("ERROR: Ошибка в подготовке запроса");
        sqlite3_close(db);
        free(json_file_content);
        return 9;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    cJSON_ArrayForEach(one_filesig_from_array, filesigs_array_from_json_file){
        cJSON* header_hex_obj = cJSON_GetObjectItem(one_filesig_from_array, "Header (hex)");
        char* header_hex = header_hex_obj->valuestring;
        if(header_hex == NULL || strlen(header_hex) == 0) continue;
        unsigned char* binary_sig = NULL;
        size_t header_hex_size = strlen(header_hex);
        binary_sig = malloc((header_hex_size / 2) * sizeof(unsigned char));
        if(binary_sig == NULL){
            printf("ERROR: Ошибка в выделении памяти для binary_sig");
            sqlite3_close(db);
            free(json_file_content);
            return 10;
        }
        size_t binary_sig_len = 0;
        result = hex_to_bytes(header_hex, binary_sig, &binary_sig_len);
        if(result != 0){
            printf("ERROR: Ошибка в выполнении ф-ции hex_to_bytes");
            sqlite3_close(db);
            free(json_file_content);
            return 11;
        }
        sqlite3_bind_blob(stmt, 3, binary_sig, binary_sig_len, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, binary_sig_len);

        cJSON* description_obj = cJSON_GetObjectItem(one_filesig_from_array, "File description");
        char* description = "Unknown";
        if(cJSON_IsString(description_obj) && (description_obj->valuestring != NULL)) description = description_obj->valuestring;
        sqlite3_bind_text(stmt, 2, description, -1, SQLITE_TRANSIENT);

        cJSON* offset_obj = cJSON_GetObjectItem(one_filesig_from_array, "Header offset");
        char* offset = "0";
        if(cJSON_IsString(offset_obj) && (offset_obj->valuestring != NULL)) offset = offset_obj->valuestring;
        long offset_val = 0;
        offset_val = parse_offset(offset);
        sqlite3_bind_int(stmt, 5, offset_val);

        cJSON* trailer_hex_obj = cJSON_GetObjectItem(one_filesig_from_array, "Trailer (hex)");
        char* trailer_hex = "(null)";
        if(cJSON_IsString(trailer_hex_obj) && (trailer_hex_obj->valuestring != NULL)) trailer_hex = trailer_hex_obj->valuestring;
        unsigned char* trailer_bytes = NULL;
        size_t trailer_hex_size = strlen(trailer_hex);
        trailer_bytes = malloc((trailer_hex_size / 2) * sizeof(unsigned char));
        if(trailer_bytes == NULL){
            printf("ERROR: Ошибка в выделении памяти для trailer_bytes");
            sqlite3_close(db);
            free(json_file_content);
            return 12;
        }
        size_t trailer_bytes_size = 0;
        result = hex_to_bytes(trailer_hex, trailer_bytes, &trailer_bytes_size);
        if(result != 0 && result != -1){
            printf("ERROR: Ошибка в выполнении ф-ции hex_to_bytes");
            sqlite3_close(db);
            free(json_file_content);
            return 13;
        }
        else if(result == -1) trailer_bytes_size = 0;
        if(trailer_bytes_size > 0) sqlite3_bind_blob(stmt, 6, trailer_bytes, trailer_bytes_size, SQLITE_TRANSIENT);
        else sqlite3_bind_null(stmt, 6);
        sqlite3_bind_int(stmt, 7, trailer_bytes_size);

        cJSON* string_of_extensions_obj = cJSON_GetObjectItem(one_filesig_from_array, "File extension");
        char* string_of_extensions = "";
        if(cJSON_IsString(string_of_extensions_obj) && (string_of_extensions_obj->valuestring != NULL)) string_of_extensions = string_of_extensions_obj->valuestring;
        if(strlen(string_of_extensions) == 0){
            string_of_extensions = "[*]";
            sqlite3_bind_text(stmt, 1, string_of_extensions, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        else{
            char* token = strtok(string_of_extensions, "|");
            while(token != NULL){
                sqlite3_bind_text(stmt, 1, token, -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
                token = strtok(NULL, "|");
            }
        }
        
        free(binary_sig);
        free(trailer_bytes);
    }

    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

    sqlite3_finalize(stmt);

    free(json_file_content);
    cJSON_Delete(json_var);
    sqlite3_close(db);
    return 0;
}