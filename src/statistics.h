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
 * calculates all map distances to each other
 * output distances [index of map][index of other map]
 *
 * NOTE map biomsVecLen must be initialized
 *      the biom array must have all the same length
 *
 * @returns 1 -> ok; 0 -> error
 */
int getAllMapDistances(struct rotaMap *allMaps, int allMapsLen, double **distances);

#endif