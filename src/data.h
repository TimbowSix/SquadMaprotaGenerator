#ifndef data_h
#define data_h

#include "config.h"
#include "rotaMap.h"
#include "rotaLayer.h"
#include "io.h"

/**
 * pointer to all maps
 */
extern rotaMap *allMaps;
/**
 * pointer to all layers
 */
extern rotaLayer *allLayers;
/**
 * pointer to all modes
 */
extern rotaMode *allModes;

/**
 * init all maps
 */
void initializeMaps(rotaConfig *conf, rotaMap *maps);

/**
 * calculating the expected distribution for every given map
 */
void getDist(rotaMap *maps, double **distances);

/**
 * parsing the mapsize in km² to a value between 0 and 1 for every given map
 */
void normalizeBiomMapSize(double **bioms);

/**
 * retrieves layers, votes and maps from a fetched input file
 * only inits name and votes
 */
void getLayers(rotaConfig *conf, rotaLayer **allLayers);

/**
 * save layers in file
 * return 0 for an error 1 for ok
 */
int saveLayers(rotaLayer *layers, int len);

/**
 * load a layers file
 *
 */
int loadLayers(rotaLayer **layers);

/**
 * removing unavailable modes/maps und recalculating probabilities if necessary
 */
void fixUnavailable(rotaConfig *conf, rotaMap *maps);

/**s
 * building a config object based on the config.json and possible overwrites
 */
void buildConfig(rotaConfig *con);

/**
 * checking if important changes in the configuration have been made
 */
int checkChanges();

#endif