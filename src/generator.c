#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "generator.h"
#include "utils.h"
#include "statistics.h"

rotaMode *chooseMode(rotaConfig *conf, mapRota *rota, rotaMode *modes, int modeLen)
{
    double sum = 1;
    // mode in mode buffer
    if (rota->modeBuffer != NULL)
    {
        return rota->modeBuffer;
    }

    // choose mode
    int choosePoolIndex = 0;
    if (rota->lastNonMainModeDistance < conf->modeDist->poolSpacing)
    {
        assert(rota->pools[0].isMainPool == 1);
        rota->lastNonMainModeDistance++;
    }
    else
    {
        // all modes

        choosePoolIndex = weightedChoose(rota->poolsProbability, rota->poolLen, &sum);
        if (choosePoolIndex > 0)
        {
            // non main mode
            rota->lastNonMainModeDistance = 0;
        }
    }
    int modeIndex = weightedChoose(rota->pools[choosePoolIndex].modeProbability, rota->pools[choosePoolIndex].len, &sum);
    return rota->pools[choosePoolIndex].modes[modeIndex];
}

rotaLayer *chooseLayer(rotaConfig *conf, rotaLayer *layers, int layerLen, double *mem)
{
    double sum = 0;
    int count = 0;
    for (int i = 0; i < layerLen; i++)
    {
        if (conf->useLayerVoteWeight)
        {

            sum += layers[i].voteWeight;
            mem[i] = layers[i].voteWeight;
        }
        else
        {

            sum += 1;
            mem[i] = 1;
        }
    }
    int c = weightedChoose(mem, layerLen, &sum);
    return &(layers[c]);
}

rotaLayer *chooseLayerFromMap(rotaConfig *conf, rotaMap *map, rotaMode *mode, double *mem, int *indexMem)
{
    int lCount = map->layerCount - map->currentLayersLockedCount;
    double sum = 0;
    int j = 0;
    for (int i = 0; i < map->layerCount; i++)
    {
        if (map->layers[i]->currentLockTime == 0 && map->layers[i]->mode->index == mode->index)
        {
            indexMem[j] = i;
            if (conf->useLayerVoteWeight == 1)
            {
                sum += map->layers[i]->voteWeight;
                mem[j] = map->layers[i]->voteWeight;
            }
            else
            {
                sum += 1;
                mem[j] = 1;
            }
            j++;
        }
    }
    int choosenIndex = indexMem[weightedChoose(mem, lCount, &sum)];
    // update layer locktime
    map->layers[choosenIndex]->currentLockTime = map->layers[choosenIndex]->lockTime;
    map->currentLayersLockedCount++;
    return map->layers[choosenIndex];
}

int getAvailableMaps(rotaMap **validMaps,
                     rotaMap *allMaps,
                     int allMapsLen,
                     rotaMode *currMode,
                     int *lockMapsCount,
                     mapRota *rota)
{
    int ret = 0;

    while (ret == 0)
    {
        ret = getValidMaps(validMaps, allMaps, allMapsLen, currMode, lockMapsCount, rota);
    }
    return ret;
}

rotaMap *chooseMap(rotaMap **validMaps, int mapsLen, rotaMode *currMode, int *lockedMapsCount, double *mem)
{
    // TODO pre allocated space for this operation

    double sum = 0;
    for (int i = 0; i < mapsLen; i++)
    {
        double a = validMaps[i]->calcMapWeight(currMode, validMaps[i]);
        assert(a != 0); // for testing
        mem[i] = a;
        sum += a;
    }
    int i = weightedChoose(mem, mapsLen, &sum);

    // set locktime
    for (int j = 0; j < validMaps[i]->neighbourCount; j++)
    {
        validMaps[i]->neighbour[j]->setLockTime(validMaps[i]->neighbour[j]);
        (*lockedMapsCount)++;
    }

    return validMaps[i];
}

void reset(rotaMap *maps, int mapLen)
{
    for (int i = 0; i < mapLen; i++)
    {
        maps[i].currentLockTime = 0;
        maps[i].resetLayerLockTime(&maps[i]);
    }
}
void initMapRota(rotaConfig *conf, mapRota **rota, rotaMode *allModes, int modesLen, int layerLen)
{
    (*rota) = malloc(sizeof(mapRota));
    (*rota)->rotation = malloc(conf->numberOfLayers * sizeof(void *));
    (*rota)->modes = malloc(conf->numberOfLayers * sizeof(void *));
    (*rota)->maps = malloc(conf->numberOfLayers * sizeof(void *));
    (*rota)->lastNonMainModeDistance = 0;
    (*rota)->poolLen = conf->modeDist->poolDistCount;
    (*rota)->poolsProbability = malloc(conf->modeDist->poolDistCount * sizeof(double));
    (*rota)->pools = malloc(conf->modeDist->poolDistCount * sizeof(rotaModePool));
    (*rota)->modeBuffer = NULL;
    (*rota)->tempModeBuffer = NULL;
    (*rota)->mem = malloc(layerLen * sizeof(double));
    (*rota)->indexMem = malloc(layerLen * sizeof(double));

    for (int i = 0; i < conf->modeDist->poolDistCount; i++)
    {
        int count = conf->modeDist->modePools[i]->modeCount;
        (*rota)->pools[i].modeProbability = malloc(count * sizeof(double));
        (*rota)->pools[i].modes = malloc(count * sizeof(void *));

        (*rota)->pools[i].len = count;

        if (strcasecmp(conf->modeDist->modePools[i]->name, "main") == 0)
        {
            (*rota)->pools[i].isMainPool = 1;
        }
        else
        {
            (*rota)->pools[i].isMainPool = 0;
        }

        for (int j = 0; j < count; j++)
        {
            (*rota)->poolsProbability[i] = conf->modeDist->poolDist[i]->probability;
            // search for mode in all modes
            int foundMode = 0;
            for (int k = 0; k < modesLen; k++)
            {
                if (strcasecmp(conf->modeDist->modePools[i]->gameMods[j]->name,
                               allModes[k].name) == 0)
                {
                    foundMode = 1;
                    (*rota)->pools[i].modes[j] = &allModes[k];
                    (*rota)->pools[i].modeProbability[j] = allModes[k].probability;
                    break;
                }
            }
            if (foundMode == 0)
            {
                printf("ERROR conf mode \"%s\", not found in allModes \n", conf->modeDist->modePools[i]->gameMods[j]->name);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void printMemColonelState(rotaMap *maps, int mapsLen)
{
    printf("NAME \r\t\t\t CurrLockTime \r\t\t\t\t\t LockedLayers \n");
    for (int i = 0; i < mapsLen; i++)
    {
        printf("%s \r\t\t\t %i \r\t\t\t\t\t %i \n", maps[i].name, maps[i].currentLockTime, maps[i].currentLayersLockedCount);
    }
}

void generateRota(rotaConfig *conf, mapRota **rota, rotaMap *maps, int mapLen, rotaMode *modes, int modeLen, int layerLen)
{
    reset(maps, mapLen);
    initMapRota(conf, rota, modes, modeLen, layerLen);

    rotaMode *currMode;
    rotaMap *currMap;
    rotaLayer *currLayer;
    rotaMap **vMaps = malloc(mapLen * sizeof(void *));
    int lockedMapsCount = 0;
    int vMapsLen = 0;

    for (int i = 0; i < conf->numberOfLayers; i++)
    {

        do
        {
            currMode = chooseMode(conf, *rota, modes, modeLen);
            vMapsLen = getAvailableMaps(vMaps, maps, mapLen, currMode, &lockedMapsCount, *rota);
        } while (vMapsLen < 0);
        (*rota)->modeBuffer = (*rota)->tempModeBuffer;
        (*rota)->tempModeBuffer = NULL;

        currMap = chooseMap(vMaps, vMapsLen, currMode, &lockedMapsCount, (*rota)->mem);
        currLayer = chooseLayerFromMap(conf, currMap, currMode, (*rota)->mem, (*rota)->indexMem);

        // add mode map layer to rota
        // printf("%s %s %s\n", currMode->name, currMap->name, currLayer->name);
        // printMemColonelState(maps, mapLen);

        (*rota)->modes[i] = currMode;
        (*rota)->maps[i] = currMap;
        (*rota)->rotation[i] = currLayer;
    }
}
