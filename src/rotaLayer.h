#ifndef RotaLayer_h
#define RotaLayer_h

#include "rotaMap.h"
#include "rotaMode.h"

typedef struct rotaLayer rotaLayer;

struct rotaLayer
{
    char *name;
    struct rotaMode *mode;
    struct rotaMap *map;
    double votes;
    int lockTime;
    int currentLockTime;
    double voteWeight;
};
#endif