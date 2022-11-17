#ifndef Rotamap_h
#define Rotamap_h

#include "rotaLayer.h"
#include "rotaMode.h"

typedef struct rotaMap rotaMap;

/**
 * index               indexes all maps in corresponding arrays
 * name                name of the map
 * layers              list of available layers
 * layerCount          count of layers
 * modes               list of all modes
 * modesCount          count of modes
 * bioms               array of double biom values
 * mapWeights          array of double indexes by mode index
 * mapVoteWeights      array of double indexes by mode index
 * distances           pointer to array of doubles indexes by map index
 * neighbour           array of rotaMaps indexes by map index
 * neighbourCount      count of neighbour
 * lockTime            reset lockTime for this map
 * currentLockTime
 * lockTimeModifier    indexes by mode index
 * distribution
 * clusterOverlap
 * mapVoteWeightSum    array of Sums by mode index
 * sigmoidValues       values for layer and map sigmoid
 *                          index   0 -> mapVote slope
 *                                  1 -> mapVote shift
 *                                  2 -> layerVote slope
 *                                  3 -> layerVote shift
 *
 *
 */
struct rotaMap
{
    int index;
    char *name;
    rotaLayer **layers;
    int layerCount;
    rotaMode **modes;
    int modeCount;
    double *biom;
    int biomLen;
    double biomVecLen;
    double *mapWeights;
    double *mapVoteWeights;
    double *distances;
    rotaMap **neighbour;
    int neighbourCount;
    int lockTime;
    int currentLockTime;
    int *lockTimeModifier;
    // for optimizer
    double distribution;
    int clusterOverlap;
    // layer lockTime
    double *mapVoteWeightSum;
    double *sigmoidValues;

    void (*lockLayer)(rotaLayer *layer);

    void (*newWeight)(rotaMode *mode, rotaMap *self);

    void (*resetLayerLockTime)(rotaMap *self);

    void (*decreaseLayerLockTime)(rotaMap *self);

    void (*addLayer)(rotaLayer *layer, rotaMap *self);

    void (*decreaseLockTime)(rotaMap *self);

    void (*setLockTime)(rotaMap *self);

    void (*calcMapVoteWeight)(rotaMode *mode, rotaMap *self);

    void (*calcLayerVoteWeight)(rotaMap *self);

    double (*calcMapWeight)(rotaMode *mode, rotaMap *self, double *params, int paramLen);
};

/**
 * initializes rotaMap struct
 * allocates needed space for rotaMap struct
 */
void newMap(rotaMap *map, int maxMapCount, int maxLayerCount, int maxModeCount);

/**
 * deletes map
 */
void delMap(rotaMap *map);

/**
 * locks a layer by
 * move the layer from linkedLayers to linkedLockedLayers
 */
void lockLayer(rotaLayer *layer);

/**
 * calculating new mapVote weight sum for a specific mode
 */
void newWeight(rotaMode *mode, rotaMap *self);

/**
 * resetting the layer lockTime for every currently locked layer, making every layer available again.
 */
void resetLayerLockTime(rotaMap *self);

/**
 * decreasing the layer lockTime by one for every locked layer.
 * re-adding the layer to the for this map available layers if no lockTime is left.
 */
void decreaseLayerLockTime(rotaMap *self);

/**
 * adding a new layer as a property of this map
 */
void addLayer(rotaLayer *layer, rotaMap *self);

/**
 * decreasing the lockTime by one
 */
void decreaseLockTime(rotaMap *self);

/**
 * setting the current lockTime of this map to default lockTime
 */
void setLockTime(rotaMap *self);

/**
 * calculating the mapVote weight for a mode
 * and saves it in the rotaMap struct
 */
void calcMapVoteWeight(rotaMode *mode, rotaMap *self);

/**
 * calculating the mapVote weight for all modes
 * and saves it in the rotaMap struct
 */
void calcAllMapVoteWeight(rotaMap *self);

/**
 * calculating layerVotes to weights
 */
void calcLayerVoteWeight(rotaMap *self);

/**
 * calculates mapWeight for given mode based on given params
 * NOTE params has to been an amount of 6
 */
double calcMapWeight(rotaMode *mode, rotaMap *self, double *params, int paramLen);

#endif