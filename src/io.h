#ifndef io_h
#define io_h

#include <json.h>

/**
 * read a json file
*/
int readJsonFile(char* path, struct json_object** object);

/**
 * writes a json file
*/
int writeJsonFile(char* path, struct json_object* object);

#endif