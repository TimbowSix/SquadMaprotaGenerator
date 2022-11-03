#ifndef RotaLayer_h
#define RotaLayer_h

#include "rotaMap.h"
#include "rotaMode.h"

struct linkedRotaLayer {
    struct rotaLayer* layer;
    struct linkedRotaLayer* next;
    struct linkedRotaLayer* before;
};


struct rotaLayer{
    char* name;
    struct rotaMode* mode;
    struct rotaMap* map;
    double votes;

    void (*init)(char* name, int nameLen, struct rotaMode* mode, struct rotaMap* map, double votes);
};

#endif