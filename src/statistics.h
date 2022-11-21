#ifndef statistics_h
#define statistics_h

#include "rotaMap.h"
#include "rotaLayer.h"
#include "rotaMode.h"
#include "generator.h"

/**
 * fills an array of indexes of valid maps to choose from
 * output format list of maps
 * if mode is in layer not available the current choosen mode
 * is written in to the mode buffer of the mapRota struct
 *
 * NOTE make sure the validMaps len is at least as big as allmaps
 *
 * @returns len of validMaps; -1 if no maps are found because of the mode
 */
int getValidMaps(
    struct rotaMap **validMaps,
    struct rotaMap *allMaps,
    int allMapsLen,
    struct rotaMode *currentMode,
    int *lockMapsCount,
    mapRota *rota);

/**
 * calculates distance between map a and b
 * based on the biom vector len value
 * and biom values (vector)
 *
 * @returns distance
 */
double getMapDistance(rotaMap *a, rotaMap *b);

#endif