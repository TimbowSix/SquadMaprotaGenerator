#include <stdio.h>
#include "io.h"


int readJsonFile(char* path, struct json_object** object){
    FILE* file = fopen(path, "r");

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file) + 1;
    rewind(file);

    char* fileBuf = (char *)malloc(fileSize);

    if(file == NULL){
        printf("File not found %s",path);
        return 0;
    }

    fread(fileBuf, sizeof(char), fileSize, file);
    fclose(file);

    enum json_tokener_error jerr;
    struct json_tokener* tok = json_tokener_new();

    (*object) = json_tokener_parse_ex(tok, fileBuf, fileSize);
    jerr = json_tokener_get_error(tok);
    if(jerr != json_tokener_success){
        printf("Json parsing error %s", jerr);
        return 0;
    }

    free(fileBuf);

    return 1;
}


int writeJsonFile(char* path, struct json_object* object){

}