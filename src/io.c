#include <stdio.h>
#include "io.h"

int readJsonFile(char* path, struct json_object* object){
    FILE* file = fopen(path, "r");

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file) + 1;
    rewind(file);

    printf("%i",fileSize);

    char* fileBuf = (char *)malloc(fileSize);

    if(file == NULL){
        printf("File not found %s",path);
        return 0;
    }

    fread(fileBuf, sizeof(char), fileSize, file);
    fclose(file);

    object = json_tokener_parse(fileBuf);


    printf("%s",json_object_to_json_string(object));
}


int writeJsonFile(char* path, struct json_object* object){

}