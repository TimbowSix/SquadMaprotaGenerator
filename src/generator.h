#ifndef generator_h
#define generator_h

#include "rotaMode.h"
#include "rotaMap.h"
#include "rotaLayer.h"

typedef struct rotaModePool
{
    int isMainPool;
    double *modeProbability;
    rotaMode **modes;
    int len;
} rotaModePool;

typedef struct mapRota
{
    rotaLayer **rotation;
    rotaMode **modes;
    rotaMap **maps;
    rotaMode *tempModeBuffer;
    rotaMode *modeBuffer;
    int lastNonMainModeDistance;
    double *poolsProbability;
    rotaModePool *pools;
    int poolLen;
} mapRota;

/**
 * Selects a random game mode based on the modes in the mode pools
 * and the corresponding probabilities set in the configuration
 */
rotaMode *chooseMode(rotaConfig *conf, mapRota *rota, rotaMode *modes, int modeLen);

/**
 * Gets a list of layers and returns a random layer from this list.
 */
rotaLayer *chooseLayer(rotaConfig *conf, rotaLayer *layers, int layerLen);

/**
 * randomly chooses a layer of a mode from a specific map
 */
rotaLayer *chooseLayerFromMap(rotaConfig *conf, rotaMap *map, rotaMode *mode);

/**
 * resets mem kernel
 */
void reset(rotaMap *maps, int mapLen);

/**
 * gets valid maps for current biom distribution
 * returns length of validMaps
 */
int getAvailableMaps(rotaMap **validMaps,
                     rotaMap *allMaps,
                     int allMapsLen,
                     rotaMode *currMode,
                     int *lockMapsCount,
                     mapRota *rota);

/**
 * calculates the new map weight and
 * chooses map from available maps
 */
rotaMap *chooseMap(rotaMap **validMaps, int mapsLen, rotaMode *currMode, int *lockedMapsCount);

/**
 * init mapRota struct, should be called before generateRota
 */
void initMapRota(rotaConfig *conf, mapRota **rota, rotaMode *allModes, int modesLen);

/**
 * for debugging
 * prints the state of maps layers
 */
void printMemColonelState(rotaMap *maps, int mapsLen);

/**
 * generates one map rota based on config and the mem kernel state(aka state of maps and layers)
 * returns length of the rotation
 */
void generateRota(rotaConfig *conf, mapRota **rota, rotaMap *maps, int mapLen, rotaMode *modes, int modeLen);

#endif