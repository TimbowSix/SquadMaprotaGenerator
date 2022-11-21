#ifndef rotaLayer_h
#define rotaLayer_h

#include "rotaMode.h"
#include "config.h"

typedef struct rotaLayer rotaLayer;

struct rotaLayer
{
    char name[MAX_LAYER_NAME_LENGTH];
    struct rotaMode *mode;
    struct rotaMap *map;
    double votes;
    int lockTime;
    int currentLockTime;
    double voteWeight;
};

typedef struct rotaLayerFileHeader rotaLayerFileHeader;
struct rotaLayerFileHeader
{
    int count;
};

/**
 * for debugging
 * prints layer in console
 */
void printLayer(rotaLayer *layer);

#endif