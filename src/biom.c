#include <stdio.h>
#include <json.h>
#include <errno.h>
#include <ctype.h>

#include "biom.h"
#include "config.h"
#include "utils.h"

int getBioms(biom **bioms)
{
    struct json_object *jBioms;
    if (readJsonFile(BIOMS_PATH, &jBioms) == 0)
    {
        printf("Error cannot read bioms.json: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int biomLen = json_object_object_length(jBioms);

    biom *b = malloc(biomLen * sizeof(biom));

    int k = 0;
    json_object_object_foreach(jBioms, mName, arr)
    {
        b[k].mapName = malloc((strlen(mName) + 1) * sizeof(char));
        strcpy(b[k].mapName, mName);
        // to lower case
        for (int i = 0; b[k].mapName[i]; i++)
        {
            b[k].mapName[i] = tolower(b[k].mapName[i]);
        }

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

void normalizeBiomMapSize(biom **bioms, int len)
{
    double max, min;
    if (len > 0)
    {
        max = (*bioms)[0].values[0];
        min = (*bioms)[0].values[0];

        for (int i = 0; i < len; i++)
        {
            if (max < (*bioms)[i].values[0])
            {
                max = (*bioms)[i].values[0];
            }
            if (min > (*bioms)[i].values[0])
            {
                min = (*bioms)[i].values[0];
            }
        }

        if ((max - min) == 0)
        {
            printf("Error, can't normalize bioms map values (div by 0)\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < len; i++)
        {
            (*bioms)[i].values[0] = ((*bioms)[i].values[0] - min) / (max - min);
        }
    }
}