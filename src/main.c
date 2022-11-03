#include <stdio.h>
#include "config.h"
#include "data.h"
#include "io.h"

int main (char** args){
    struct json_object* object;
    readJsonFile("../config.json", object);
}