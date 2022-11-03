#include <stdio.h>
#include "config.h"
#include "data.h"
#include "io.h"
#include <json.h>

int main (char** args){
    struct json_object* object;
    readJsonFile("../config.json", &object);

    printf("%s", json_object_to_json_string(object));
}