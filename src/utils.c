#include <stdio.h>
#include <math.h>

#include "utils.h"

double calcVectorLength(double *ele, int countOfElements)
{
    double tempSum = 0;
    for (int i = 0; i < countOfElements; i++)
    {
        tempSum += pow(ele[i], 2);
    }
    return sqrt(tempSum);
}

double calcVectorDotProduct(double *vecA, double *vecB, int vecLen)
{
    double sum = 0;
    for (int i = 0; i < vecLen; i++)
    {
        sum += vecA[i] * vecB[i];
    }
    return sum;
}

void normalize(double *arr, int len, double *sum)
{
    if (len <= 0)
    {
        return;
    }

    double s = 0;
    if (sum != NULL)
    {
        s = *sum;
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            s += arr[i];
        }
    }
    if (s == 0)
    {
        double temp = 1 / len;
        for (int i = 0; i < len; i++)
        {
            arr[i] = temp;
        }
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            arr[i] /= s;
        }
    }
}

double sigmoid(double x, double slope, double shift)
{
    double arg = slope * (x + shift);
    return 1 / (1 + exp(-arg));
}

int getJsonIntValue(struct json_object *ob, char *key, int *value)
{
    struct json_object *v;
    json_object_object_get_ex(ob, key, &v);
    if (json_object_get_type(v) == json_type_int)
    {
        *value = json_object_get_int(v);
        return 1;
    }
    else
    {
        printf("Error %s is not a int\n", key);
        exit(EXIT_FAILURE);
        return 0;
    }
}

int getJsonStringValue(struct json_object *ob, char *key, char *value[], int maxLen)
{
    struct json_object *v;
    json_object_object_get_ex(ob, key, &v);
    if (json_object_get_type(v) == json_type_string)
    {
        const char *temp = json_object_get_string(v);
        if (strlen(temp) + 1 > maxLen)
        {
            printf("Error String to big in getJsonStringValue\n");
            exit(EXIT_FAILURE);
            return 0;
        }
        strcpy(*value, temp);
        return 1;
    }
    else
    {
        printf("Error %s is not a string\n", key);
        exit(EXIT_FAILURE);
        return 0;
    }
}

int getJsonDoubleValue(struct json_object *ob, char *key, double *value)
{
    struct json_object *v;
    json_object_object_get_ex(ob, key, &v);
    if (json_object_get_type(v) == json_type_double)
    {
        *value = json_object_get_double(v);
        return 1;
    }
    else
    {
        printf("Error %s is not a double\n", key);
        exit(EXIT_FAILURE);
        return 0;
    }
}

int getJsonBoolValue(struct json_object *ob, char *key, int *value)
{
    struct json_object *v;
    json_object_object_get_ex(ob, key, &v);
    if (json_object_get_type(v) == json_type_boolean)
    {
        *value = json_object_get_boolean(v);
        return 1;
    }
    else
    {
        printf("Error %s is not a boolean\n", key);
        exit(EXIT_FAILURE);
        return 0;
    }
}