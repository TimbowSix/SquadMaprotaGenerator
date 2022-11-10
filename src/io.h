#ifndef io_h
#define io_h

#include <json.h>
#include "io.h"

/**
 * init components globally
 */
void ioInit();

/**
 * read a json file
 */
int readJsonFile(char *path, struct json_object **object);

/**
 * writes a json file
 */
int writeJsonFile(char *path, struct json_object *object);

/**
 * read mapvotes from webpage
 */
int getLayerData(char *url, struct json_object **object);

/**
 * curl result
 */
struct curlRes
{
    char *ptr;
    size_t len;
};

/**
 * init curlRes struct
 */
void initCurlRes(struct curlRes *r);

/**
 * callback function for curl
 */
size_t curlWriteData(void *buffer, size_t size, size_t nmemb, struct curlRes *r);

#endif