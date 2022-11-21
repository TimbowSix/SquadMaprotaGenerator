#ifndef utils_h
#define utils_h

#include <json.h>
#include <string.h>

#include "config.h"

/**
 * calc len of vector aka array
 */
double calcVectorLength(double *ele, int countOfElements);

/**
 * calc dot product of two vectors
 */
double calcVectorDotProduct(double *vecA, double *vecB, int vecLen);

/**
 * normalize an array of double values
 * takes sum, set it NULL if not given
 */
void normalize(double *arr, int len, double *sum);

/**
 * calculate y values of the sigmoid function
 */
double sigmoid(double x, double slope, double shift);

/**
 * gets an integer by key of an json_object struct
 * returns 1 for success
 */
int getJsonIntValue(struct json_object *ob, char *key, int *value);

/**
 * gets an string by key of an json_object struct
 * returns 1 for success
 */
int getJsonStringValue(struct json_object *ob, char *key, char *value[], int maxLen);

/**
 * gets an double by key of an json_object struct
 * returns 1 for success
 */
int getJsonDoubleValue(struct json_object *ob, char *key, double *value);

/**
 * gets an bool by key of an json_object struct
 * returns 1 for success
 */
int getJsonBoolValue(struct json_object *ob, char *key, int *value);

/**
 * for saving map dist
 */
typedef struct mapDistributionForSaving
{
    char mapName[MAX_MAP_NAME_LENGTH];
    char modeName[MAX_MODE_NAME_LENGTH];
    double dist;
} mapDistributionForSaving;

/**
 * weighted choice on an array with weights
 * WARNING manipulates arr!
 * return choosen index
 */
int weightedChoose(double *arr, int len, double *sum);

/**
 * init rand() with seed
 * NOTE has to be call befor weightedChoose
 */
void initRandom(unsigned int seed);

#endif