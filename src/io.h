#ifndef io_h
#define io_h

#include <json.h>

void readFile(char* path, int len, int* file);

void writeFile(char* path, int len, int* file);

void readJsonFile(int* file, struct json_object* object);

void writeJsonFile(struct json_object* object);

#endif