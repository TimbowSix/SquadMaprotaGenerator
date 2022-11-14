#include <stdio.h>
#include <json.h>

#include "biom.h"
#include "config.h"
#include "utils.h"

int getBioms(biom **bioms)
{
    struct json_object *jBioms;
    if (readJsonFile(BIOMS_PATH, &jBioms) == 0)
    {
        exit(EXIT_FAILURE);
    }

    int biomLen = json_object_object_length(jBioms);

    biom *b = malloc(biomLen * sizeof(biom));

    int k = 0;
    json_object_object_foreach(jBioms, mName, arr)
    {
        b[k].mapName = malloc((strlen(mName) + 1) * sizeof(char));
        strcpy(b[k].mapName, mName);

        int arrLen = json_object_array_length(arr);
        b[k].values = malloc(arrLen * sizeof(double));
        b[k].len = arrLen;

        for (int i = 0; i < arrLen; i++)
        {
            struct json_object *biomVal = json_object_array_get_idx(arr, i);
            if (json_object_get_type(biomVal) != json_type_double)
            {
                printf("Error bioms: a biom values in \"%s\" is not a double\n", mName);
                exit(EXIT_FAILURE);
            }
            b[k].values[i] = json_object_get_double(biomVal);
        }
        k++;
    }
    *bioms = b;
    return biomLen;
}

void delBioms(biom *b, int len)
{
    for (int i = 0; i < len; i++)
    {
        free(b[i].mapName);
        free(b[i].values);
    }
    free(b);
}

void printBioms(biom *bioms, int len)
{
    printf("Mapname \t\t Biom values\n");
    for (int i = 0; i < len; i++)
    {
        printf("%s\t\t [", bioms[i].mapName);
        for (int j = 0; j < bioms[i].len; j++)
        {
            printf(" %f,", bioms[i].values[j]);
        }
        printf("] \n");
    }
}