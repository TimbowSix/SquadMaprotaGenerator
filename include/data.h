#ifndef DATA
#define DATA

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "rotaMap.h"
#include "rotaLayer.h"


void initialize_maps(struct config*, struct rotaMap* maps);

void get_dist(struct rotaMap* maps);

#endif