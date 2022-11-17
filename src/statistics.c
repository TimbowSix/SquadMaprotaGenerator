#include <stdio.h>
#include <math.h>

#include "statistics.h"
#include "utils.h"

int getValidMaps(
    struct rotaMap **validMaps,
    struct rotaMap *allMaps,
    int allMapsLen,
    struct rotaMap *lastChoosenMap,
    struct rotaMode *currentMode)
{
    if (lastChoosenMap == NULL)
    {
        return -1;
    }

    // decrease lockTime of map
    for (int i = 0; i < allMapsLen; i++)
    {
        allMaps[i].decreaseLockTime(&allMaps[i]);
    }

    // set lockTime of neighbour and choosen map
    for (int i = 0; i < lastChoosenMap->neighbourCount; i++)
    {
        lastChoosenMap->neighbour[i]->setLockTime((lastChoosenMap->neighbour[i]));
    }

    int counter = 0;
    // get valid maps aka maps with lockTime 0
    for (int i = 0; i < allMapsLen; i++)
    {
        if ((allMaps[i].lockTime - allMaps[i].lockTimeModifier[currentMode->index]) <= 0)
        {
            validMaps[counter] = &allMaps[i];
            counter++;
        }
    }

    return counter;
}

double getMapDistance(rotaMap *a, rotaMap *b)
{
    if (a->index == b->index)
        return 0.0;
    return acos(calcVectorDotProduct(a->biom, b->biom, a->biomLen) / (a->biomVecLen * b->biomVecLen));
}