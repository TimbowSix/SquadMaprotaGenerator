#ifndef Rotamap_h
#define Rotamap_h

#include "rotaLayer.h"
#include "rotaMode.h"

/**
 * index               indexes all maps in conresponding arrays
 * name                name of the map
 * linkedLayers        linked list of available layers
 * bioms               array of double biom values
 * mapWeights          array of double indexes by mode index
 * mapVoteWeights      array of double indexes by mode index
 * distances           pointer to array of doubles indexes by map index
 * neighbour           array of rotaMaps indexes by map index
 * neighbourCount      count of neighbour
 * lockTime            reset lockTime for this map
 * currentLockTime
 * lockTimeModifier    indexes by mode index
 * voteWeightsByMode   indexes by mode index
 * distribution
 * clusterOverlap
 * mapVoteWeightSum    array of Sums by mode index
 * layerLockTime
 * linkedLockedLayers  linked list of unavailable layer
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
    int index; // index of map in arrays
    char *name;
    struct rotaLayer **linkedLayers; // list of layers
    double *biom;                    // array of biom values
    int biomLen;                     // len of bioms array
    double biomVecLen;               // vector len of the bioms vector
    double *mapWeights;              // array of mapWeights by mode
    double *mapVoteWeights;          // array of map vote weights by mode
    double **distances;              // pointer to array of map distances
    struct rotaMap *neighbour;       // array of map neighbour
    int neighbourCount;              //
    int lockTime;                    //
    int currentLockTime;             //
    int *lockTimeModifier;           // array of modifiers by mode
    double *voteWeightsByMode;       // array of voteWeights by mode
    // for optimizer
    double distribution; //
    int clusterOverlap;  //
    // layer lockTime
    double *mapVoteWeightSum;        // array of Sums by mode
    int layerLockTime;               //
    struct rotaLayer **lockedLayers; //
    double *sigmoidValues;           //

    /**
     * locks a layer by
     * move the layer from linkedLayers to linkedLockedLayers
     */
    void (*lockLayer)(struct rotaLayer *layer, struct rotaMap *self);

    /**
     * calculating new mapVote weight sum for a specific mode
     */
    void (*new_weight)(struct rotaMode *mode, struct rotaMap *self);

    /**
     * resetting the layer lockTime for every currently locked layer, making every layer available again.
     */
    void (*resetLayerLockTime)(struct rotaMap *self);

    /**
     * decreasing the layer lockTime by one for every locked layer.
     * re-adding the layer to the for this map available layers if no lockTime is left.
     */
    void (*decrease_layer_lock_time)(struct rotaMap *self);

    /**
     * adding a new layer as a property of this map
     */
    void (*add_layer)(struct rotaLayer *layer, struct rotaMap *self);

    /**
     * decreasing the locktime by one
     */
    void (*decreaseLockTime)(struct rotaMap *self);

    /**
     * setting the current lockTime of this map to default lockTime
     */
    void (*resetLockTime)(struct rotaMap *self);

    /**
     * calculating the mapVote weight for a mode
     * and saves it in the rotaMap struct
     */
    void (*addMapVoteWeight)(struct rotaMode *mode, struct rotaMap *self);

    /**
     * calculating layerVotes to weights
     */
    void (*calcVoteWeightByMode)(struct rotaMode *mode, struct rotaMap *self);

    /**
     * calculates mapWeight for given mode based on given params
     */
    void (*calcMapWeight)(struct rotaMode *mode, double *params);
};

#endif