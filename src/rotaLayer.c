#include <stdio.h>

#include "rotaLayer.h"

void printLayer(rotaLayer *layer)
{
    printf("%s %f %i %i %f\n", layer->name, layer->votes, layer->lockTime, layer->currentLockTime, layer->voteWeight);
}