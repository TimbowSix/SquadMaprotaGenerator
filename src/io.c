#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "io.h"

void ioInit()
{
    curl_global_init(CURL_GLOBAL_ALL | CURL_GLOBAL_SSL);
}

int readJsonFile(char *path, struct json_object **object)
{
    FILE *file = fopen(path, "r");

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file) + 1;
    rewind(file);

    char *fileBuf = (char *)malloc(fileSize);

    if (file == NULL)
    {
        printf("File not found %s", path);
        return 0;
    }

    fread(fileBuf, sizeof(char), fileSize, file);
    fclose(file);

    enum json_tokener_error jerr;
    struct json_tokener *tok = json_tokener_new();

    (*object) = json_tokener_parse_ex(tok, fileBuf, fileSize);
    jerr = json_tokener_get_error(tok);
    if (jerr != json_tokener_success)
    {
        printf("Json parsing error %s", jerr);
        return 0;
    }

    free(fileBuf);

    return 1;
}

int writeJsonFile(char *path, struct json_object *object)
{

    const char *jsonString = json_object_to_json_string_ext(object, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_PRETTY_TAB | JSON_C_TO_STRING_SPACED);

    FILE *file = fopen(path, "w");
    size_t w = fwrite(jsonString, 1, strlen(jsonString), file);
    if (w != strlen(jsonString))
    {
        printf("error");
        fclose(file);
        return 1;
    }
    fclose(file);

    return 0;
}

int getLayerData(char *url, struct json_object **object)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        CURLcode res;

        struct curlRes r;
        initCurlRes(&r);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            printf("Error curl %i %s", res, curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);

        enum json_tokener_error jerr;
        struct json_tokener *tok = json_tokener_new();

        (*object) = json_tokener_parse_ex(tok, r.ptr, r.len);
        jerr = json_tokener_get_error(tok);
        if (jerr != json_tokener_success)
        {
            printf("Json parsing error %s", jerr);
            return 0;
        }
    }

    return 0;
}

size_t curlWriteData(void *buffer, size_t size, size_t nmemb, struct curlRes *r)
{
    size_t new_len = r->len + size * nmemb;
    r->ptr = realloc(r->ptr, new_len + 1);
    if (r->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(r->ptr + r->len, buffer, size * nmemb);
    r->ptr[new_len] = '\0';
    r->len = new_len;

    return size * nmemb;
}

void initCurlRes(struct curlRes *r)
{
    r->len = 0;
    r->ptr = malloc(r->len + 1);
    if (r->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    r->ptr[0] = '\0';
}