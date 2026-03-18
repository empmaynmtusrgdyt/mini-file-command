#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int main(){
    sqlite3 *db;
    int result = sqlite3_open("magic.db", &db);
    if(result != SQLITE_OK){
        printf("ERROR: База данных не открыта");
        return 1;
    }
    result = sqlite3_close(db);
    if(result != SQLITE_OK){
        printf("ERROR: База данных не закрыта");
        return 2;
    }
    return 0;
}