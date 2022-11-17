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
 * init all maps, layers, modes etc
 */
void initialize(rotaConfig *conf, rotaMap **maps, rotaLayer **layers, rotaMode **modes);

/**
 * calculating the expected distribution for every given map and saves it
 */
void saveDist(rotaMap *maps, int mapLen, char *filename);

/**
 * retrieves layers, votes and maps from a fetched input file
 * only inits name and votes#
 * returns count of layers
 */
int getLayers(rotaConfig *conf, rotaLayer **allLayers, rotaMode **modes, int modeCount);

/**
 * save layers in file
 * return 0 for an error 1 for ok
 */
int saveLayers(rotaLayer **layers, int len);

/**
 * load a layers file
 * returns count of layers, error -1
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

/**
 * return mode count
 */
int getModeCount(rotaConfig *conf);

#endif