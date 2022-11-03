#ifndef data_h
#define data_H

#include "config.h"
#include "rotaMap.h"
#include "rotaLayer.h"
#include "io.h"

/**
 * pointer to all maps
*/
struct rotaMap* allMaps;
/**
 * pointer to all layers
*/
struct rotaLayer* allLayers;
/**
 * pointer to all modes
*/
struct rotaMode* allModes;

/**
 * init all maps
*/
void initializeMaps(struct rotaConfig* conf, struct rotaMap* maps);


/**
 * calculating the expected distribution for every given map
*/
void getDist(struct rotaMap* maps, double** distances);

/**
 * parsing the mapsize in km² to a value between 0 and 1 for every given map
*/
void normaliseBiomMapSize(double** bioms);

/**
 * retrieves layers, votes and maps from a fetched input file
*/
void getLayers(struct rotaConfig* conf, struct rotaLayer* allLayers);

/**
 * removing unavailable modes/maps und recalculating probabilities if necessary
*/
void fixUnavailables(struct rotaConfig* conf, struct rotaMap* maps);

/**
 * building a config object based on the config.json and possible overwrites
*/
void buildConfig(struct rotaConfig* con);

/**
 * checking if important changes in the configuration have been made
*/
int checkChanges();

#endif