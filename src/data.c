#include <stdio.h>
#include <json.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "data.h"
#include "io.h"
#include "biom.h"
#include "rotaMap.h"
#include "statistics.h"
#include "utils.h"

void initialize(rotaConfig *conf, rotaMap **maps, rotaLayer **layers, rotaMode **modes)
{
    biom *bioms;
    weightParam *params;
    int paramLen;
    int len;

    // get weight params
    paramLen = loadWeightParams(&params, WEIGHT_PARAMS_PATH);

    // bioms (map names)
    len = getBioms(&bioms);
    normalizeBiomMapSize(&bioms, len);
    // modes
    int modeCount = getModeCount(conf);
    modeCount++; // for seed mode
    *modes = malloc(modeCount * sizeof(rotaMode));

    (*modes)[0].index = 0;
    char *seedMode = "seed";
    (*modes)[0].name = seedMode;

    int modeIndex = 1;
    for (int i = 0; i < conf->modeDist->poolCount; i++)
    {
        for (int j = 0; j < conf->modeDist->modePools[i]->modeCount; j++)
        {
            (*modes)[modeIndex].index = modeIndex;
            (*modes)[modeIndex].name = malloc((strlen(conf->modeDist->modePools[i]->gameMods[j]->name) + 1) * sizeof(char));
            strcpy((*modes)[modeIndex].name, conf->modeDist->modePools[i]->gameMods[j]->name);
            // to lower case
            for (int k = 0; (*modes)[modeIndex].name[k]; k++)
            {
                (*modes)[modeIndex].name[k] = tolower((*modes)[modeIndex].name[k]);
            }
            // search for params by mode name

            for (int k = 0; k < paramLen; k++)
            {
                // TODO
            }
            modeIndex++;
        }
    }
    // layers
    int layerCount = getLayers(conf, layers, modes, modeCount);

    // init maps

    *maps = malloc(len * sizeof(rotaMap));
    for (int i = 0; i < len; i++)
    {
        newMap(&(*maps)[i], len, layerCount, modeCount);
        (*maps)[i].index = i;
        (*maps)[i].name = malloc((strlen(bioms[i].mapName) + 1) * sizeof(char));
        strcpy((*maps)[i].name, bioms[i].mapName);
        (*maps)[i].biom = bioms[i].values;
        (*maps)[i].biomLen = bioms[i].len;
        (*maps)[i].biomVecLen = calcVectorLength((*maps)[i].biom, (*maps)[i].biomLen);
        (*maps)[i].sigmoidValues[0] = conf->mapVoteSlope;
        (*maps)[i].sigmoidValues[1] = conf->mapVoteShift;
        (*maps)[i].sigmoidValues[2] = conf->layerVoteSlope;
        (*maps)[i].sigmoidValues[3] = conf->layerVoteShift;

        // add layers
        // if mapname is inside layer name
        int noLayer = 1;
        int sLen = strlen((*maps)[i].name);
        char *temp = malloc((sLen + 2) * sizeof(char));
        strcpy(temp, (*maps)[i].name);
        temp[sLen] = '_';
        temp[sLen + 1] = '\0';
        int tempLen = strlen(temp) - 1;

        for (int j = 0; j < layerCount; j++)
        {
            char *r = strstr((*layers)[j].name, temp);

            if (r)
            {
                (*maps)[i].addLayer(&(*layers)[j], &(*maps)[i]);
                noLayer = 0;
            }
        }
        free(temp);

        if (noLayer == 1)
        {
            printf("WARNING: No layers available for map %s\n", (*maps)[i].name);
        }

        // pre-calculate layer votes weights
        (*maps)[i].calcLayerVoteWeight(&((*maps)[i]));
    }

    // init map distance

    for (int i = 0; i < len; i++)
    {
        (*maps)[i].distances = malloc(len * sizeof(double));
        for (int j = 0; j < len; j++)
        {
            (*maps)[i].distances[j] = getMapDistance(&((*maps)[i]), &((*maps)[j]));
        }
    }

    // set neighbour & calc map vote weights

    double *mapVoteWeightsSum = malloc(modeCount * sizeof(double));

    for (int i = 0; i < modeCount; i++)
    {
        mapVoteWeightsSum[i] = 0;
    }

    for (int i = 0; i < len; i++)
    {
        (*maps)[i].neighbourCount = 0;
        for (int j = 0; j < len; j++)
        {
            if ((*maps)[i].distances[j] < conf->minBiomDistance)
            {
                (*maps)[i].neighbour[(*maps)[i].neighbourCount] = &((*maps)[j]);
                (*maps)[i].neighbourCount++;
            }
        }

        // calc map weights
        calcAllMapVoteWeight(&((*maps)[i]));
        for (int j = 0; j < (*maps)[i].modeCount; j++)
        {
            if ((*maps)[i].modes[j] != NULL)
            {
                mapVoteWeightsSum[(*maps)[i].modes[j]->index] += (*maps)[i].mapVoteWeightSum[j];
            }
        }
    }
    // summed up map votes to maps
    for (int i = 0; i < len; i++)
    {
        (*maps)[i].mapVoteWeightSum = mapVoteWeightsSum;
        (*maps)[i].clusterOverlap = 0; // TODO cluster overlap
    }
    if (conf->saveExpectedMapDist == 1)
    {
        saveDist(*maps, len, CURRENT_MAP_DIST_PATH);
    }
}

void saveDist(rotaMap *maps, int mapLen, char *filename)
{
    int space = 0;
    for (int i = 0; i < mapLen; i++)
    {
        for (int j = 0; j < maps[i].modeCount; j++)
        {
            if (maps[i].modes != NULL)
            {
                space++;
            }
        }
    }

    mapDistributionForSaving *dist = malloc(space * sizeof(mapDistributionForSaving));

    space = 0;
    for (int i = 0; i < mapLen; i++)
    {
        for (int j = 0; j < maps[i].modeCount; j++)
        {
            if (maps[i].modes[j] != NULL)
            {
                strcpy(dist[space].mapName, maps[i].name);
                strcpy(dist[space].modeName, maps[i].modes[j]->name);
                dist[space].dist = maps[i].mapVoteWeights[j] / maps[i].mapVoteWeightSum[j];
                space++;
            }
        }
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("WARNING map distribution can't be saved %s \n", strerror(errno));
        return;
    }
    for (int i = 0; i < space; i++)
    {
        fwrite(&(dist[i]), sizeof(mapDistributionForSaving), 1, file);
    }
    fclose(file);
}

int getLayers(rotaConfig *conf, rotaLayer **allLayers, rotaMode **modes, int modeCount)
{
    if (conf->updateLayer == 1)
    {
        json_object *jLayers;
        getLayerData(conf->layerVoteApiUrl, &jLayers);

        json_object *layerNames;
        json_object *mapVotes;

        layerNames = json_object_object_get(jLayers, "AxisLabels");
        int len = json_object_array_length(layerNames);
        *allLayers = malloc(len * sizeof(rotaLayer));

        // votes & status
        json_object *arr = json_object_object_get(jLayers, "DataSet");
        json_object *layerStatus = json_object_array_get_idx(arr, 2);
        json_object *upArr = json_object_array_get_idx(arr, 0);
        json_object *downArr = json_object_array_get_idx(arr, 1);

        if (json_object_array_length(layerNames) != json_object_array_length(upArr) || json_object_array_length(layerNames) != json_object_array_length(downArr) || json_object_array_length(layerNames) != json_object_array_length(layerStatus))
        {
            printf("Error while getting Layers: layer length dont match to attributes");
            exit(EXIT_FAILURE);
        }

        int k = 0;
        for (int i = 0; i < len; i++)
        {
            if (json_object_get_int(json_object_array_get_idx(layerStatus, i)) == 0)
            {
                const char *n = json_object_get_string(json_object_array_get_idx(layerNames, i));
                if (strlen(n) > MAX_LAYER_NAME_LENGTH)
                {
                    printf("a layer name cannot be over %i chars", MAX_LAYER_NAME_LENGTH);
                    exit(EXIT_FAILURE);
                }
                strcpy((*allLayers)[k].name, n);
                // to lower
                for (int l = 0; (*allLayers)[k].name[l]; l++)
                {
                    (*allLayers)[k].name[l] = tolower((*allLayers)[k].name[l]);
                }

                (*allLayers)[k].votes = (double)(json_object_get_int(json_object_array_get_idx(upArr, i)) + json_object_get_int(json_object_array_get_idx(downArr, i)));

                // mode for layer
                int modeFound = 0;
                for (int j = 0; j < modeCount; j++)
                {
                    int tempLen = strlen((*modes)[j].name);
                    char *temp = malloc((tempLen + 2) * sizeof(char));
                    strcpy(temp, (*modes)[j].name);
                    temp[tempLen] = '_';
                    temp[tempLen] = '\0';

                    char *r = strstr((*allLayers)[k].name, temp);

                    if (r)
                    {
                        // mode found
                        (*allLayers)[k].mode = &(*modes)[j];
                        modeFound = 1;
                        break;
                    }
                }
                if (modeFound == 1)
                {
                    k++;
                }
            }
        }

        // write file
        saveLayers(allLayers, k);
        return k;
    }
    else
    {
        // read from file
        int layerCount = loadLayers(allLayers);
        if (layerCount < 0)
        {
            printf("ERROR while loading, cannot continue\n");
            exit(EXIT_FAILURE);
        }
        return layerCount;
    }
}

int saveLayers(rotaLayer **layers, int len)
{
    FILE *file;

    file = fopen(LAYER_PATH, "w");
    if (file == NULL)
    {
        printf("ERROR can't save layer file: %s", strerror(errno));
        return 0;
    }
    rotaLayerFileHeader fileHeader = {.count = len};
    fwrite(&fileHeader, sizeof(rotaLayerFileHeader), 1, file);
    for (int i = 0; i < len; i++)
    {
        fwrite(&(*layers)[i], sizeof(rotaLayer), 1, file);
    }
    fclose(file);
    return 1;
}

int loadLayers(rotaLayer **layers)
{
    FILE *file;
    file = fopen(LAYER_PATH, "r");
    if (file == NULL)
    {
        printf("ERROR loading layer file %s", strerror(errno));
        return -1;
    }

    rotaLayerFileHeader fileHeader;
    fread(&fileHeader, sizeof(rotaLayerFileHeader), 1, file);
    (*layers) = malloc(fileHeader.count * sizeof(rotaLayer));

    int i = 0;
    while (fread(&((*layers)[i]), sizeof(rotaLayer), 1, file))
    {
        (*layers)[i].mode = NULL;
        (*layers)[i].map = NULL;
        i++;
    }
    fclose(file);
    return i;
}

void fixUnavailable(rotaConfig *conf, rotaMap *maps)
{
}

void buildConfig(rotaConfig *con)
{
}

int checkChanges()
{
}

int getModeCount(rotaConfig *conf)
{
    int modeCount = 0;
    for (int i = 0; i < conf->modeDist->poolCount; i++)
    {
        for (int j = 0; j < conf->modeDist->modePools[i]->modeCount; j++)
        {
            modeCount++;
        }
    }
    return modeCount;
}

int loadWeightParams(weightParam **params, char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error can't open weight params %s\n", strerror(errno));
        return 0;
    }
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file) / sizeof(weightParam);
    rewind(file);

    (*params) = malloc(size * sizeof(weightParam));
    int i = 0;
    while (fread(&((*params)[i]), sizeof(weightParam), 1, file))
    {
        i++;
    }
    fclose(file);
    return i;
}