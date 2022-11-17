#ifndef generator_h
#define generator_h

#include "rotaMode.h"
#include "rotaMap.h"
#include "rotaLayer.h"

typedef struct mapRota
{
    rotaLayer *rotation;
    rotaMode *modes;
    rotaMap *maps;

    rotaMode *buffer;
} mapRota;

#endif