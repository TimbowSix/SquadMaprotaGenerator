#ifndef statistics_h
#define statistics_h

#include "rotaMap.h"
#include "rotaLayer.h"
#include "rotaMode.h"

/**
 * fills an array of indexes of valid maps to choose from
 * output format list of maps
 *
 * NOTE make sure the validMaps len is at least as big as allmaps
 *
 * @returns len of validMaps; -1 -> error
 */
int getValidMaps(struct rotaMap **validMaps,
                 struct rotaMap *allMaps,
                 int allMapsLen,
                 struct rotaMap *lastChoosenMap,
                 struct rotaMode *currentMode);

/**
 * calculates distance between map a and b
 * based on the biom vector len value
 * and biom values (vector)
 *
 * @returns distance
 */
double getMapDistance(rotaMap *a, rotaMap *b);

#endif