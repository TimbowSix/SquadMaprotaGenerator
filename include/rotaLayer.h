#ifndef ROTALAYER
#define ROTALAYER

#include "rotaMap.h"

struct rotaLayer{
    char* name;
    char* mode;
    struct rotaMap* map;
    double votes;
};

#endif