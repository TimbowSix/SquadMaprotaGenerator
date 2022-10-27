#ifndef ROTAMAP
#define ROTAMAP

#include "rotaLayer.h"

struct rotaMap{
    char* name;
    struct rotaLayer* layer;
    double* bioms; // array of biom values
    double* mapvoteWeights;
};

#endif