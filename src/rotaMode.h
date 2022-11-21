#ifndef RotaMode_h
#define RotaMode_h

#include "config.h"

typedef struct rotaMode rotaMode;

struct rotaMode
{
    int index;
    char *name;
    double probability;
    double weightParams[WEIGHT_PARAMS_COUNT];
};

#endif