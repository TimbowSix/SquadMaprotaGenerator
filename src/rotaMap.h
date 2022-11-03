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
 * neighbours          array of rotaMaps indexes by map index
 * neighbourcount      count of neighbours
 * locktime            reset locktime for this map
 * currentlocktime
 * lockTimeModifier    indexes by mode index
 * voteWeightsByMode   indexes by mode index
 * distribution
 * clusterOverlap
 * mapvoteWeightSum    array of Sums by mode index
 * layerLocktime
 * linkedLockedLayers  linked list of unavailable layer
 * sigmoidValues       values for layer and map sigmoid
 *                          index   0 -> mapvote slope
 *                                  1 -> mapvote shift
 *                                  2 -> layervote slope
 *                                  3 -> layervote shift
 *
 *
*/

struct rotaMap{
    int index;                      // index of map in arrays
    char* name;
    struct linkedRotaLayer* linkedLayers;   //linked list of layers
    double* bioms;                  // array of biom values
    double* mapWeights;             // array of mapweights by mode
    double* mapVoteWeights;         // array of map vote weights by mode
    double** distances;             // pointer to array of map distances
    struct rotaMap* neighbours;     // array of map neighbours
    int neightbourCount;
    int locktime;
    int currentLocktime;
    int* lockTimeModifier;           // array of modifiers by mode
    double* voteWeightsByMode;       // array of voteWeights by mode
    //for optimizer
    double distribution;
    int clusterOverlap;
    //layer locktime
    double* mapvoteWeightSum;       // array of Sums by mode
    int layerLocktime;
    struct linkedRotaLayer* linkedLockedLayers;
    double* sigmoidValues;


    /**
     * lockes a layer by
     * move the layer from linkedLayers to linkedLockedLayers
    */
    void (*lockLayer)(struct rotaLayer* layer, struct rotaMap* self);

    /**
     * calculating new mapvote weight sum for a specific mode
    */
    void (*new_weight)(struct rotaMode* mode, struct rotaMap* self);

    /**
     * resetting the layer locktime for every currently locked layer, making every layer available again.
    */
    void (*resetLayerLocktime)(struct rotaMode* self);

    /**
     * decreasing the layer locktime by one for every locked layer.
     * re-adding the layer to the for this map available layers if no locktime is left.
    */
    void (*decrease_layer_lock_time)(struct rotaMode* self);

    /**
     * adding a new layer as a property of this map
    */
    void (*add_layer)(struct rotaLayer* layer, struct rotaMap* self);

    /**
     * decreasing the locktime by one
    */
    void (*decreaseLocktime)(struct rotaMap* self);

    /**
     * setting the current locktime of this map to default locktime
    */
    void (*resetLocktime)(struct rotaMap* self);

    /**
     * calculating the mapvote weight for a mode
     * and saves it in the rotaMap struct
    */
    void (*addMapvoteWeight)(struct rotaMode* mode, struct rotaMap* self);

    /**
     * calculating layervotes to weights
    */
    void (*calcVoteWeightByMode)(struct rotaMode* mode, struct rotaMap* self);

    /**
     * calculates mapweight for given mode based on given params
    */
    void (*calcMapWeight)(struct rotaMode* mode, double* params);
};

#endif