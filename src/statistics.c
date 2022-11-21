#include <stdio.h>
#include <math.h>

#include "statistics.h"
#include "utils.h"
#include "generator.h"

int getValidMaps(
    struct rotaMap **validMaps,
    struct rotaMap *allMaps,
    int allMapsLen,
    struct rotaMode *currentMode,
    int *lockMapsCount,
    mapRota *rota)
{
    int counter = 0;
    // get valid maps aka maps with lockTime 0
    int mapsFound = 0;

    for (int i = 0; i < allMapsLen; i++)
    {
        if ((allMaps[i].currentLockTime - allMaps[i].lockTimeModifier[currentMode->index]) <= 0)
        {
            // check if mode is available
            for (int j = 0; j < allMaps[i].layerCount; j++)
            {
                if (allMaps[i].layers[j]->currentLockTime == 0 && allMaps[i].layers[j]->mode->index == currentMode->index)
                {
                    validMaps[counter] = &allMaps[i];
                    counter++;
                    break;
                }
            }
            mapsFound = 1;
        }

        // decrease lockTime of map and layer
        allMaps[i].decreaseLayerLockTime(&allMaps[i]);

        if (allMaps[i].currentLockTime > 0)
        {
            allMaps[i].decreaseLockTime(&allMaps[i]);
            if (allMaps[i].currentLockTime == 0)
            {
                (*lockMapsCount)--;
            }
        }
    }

    // TODO NOTE locktime wird reduziert obwohl ein mode nicht gefunden werden kann und damit auch keine maps zurückgegeben werden. so richtig ?

    if (counter == 0 && mapsFound > 0)
    {
        // maps found but no mode
        rota->tempModeBuffer = currentMode;
        if (currentMode->index == 0 || currentMode->index == 1)
        {
            printMemColonelState(allMaps, allMapsLen);
            printf("ERROR all Layers Locked cannot continue\n");
            exit(EXIT_FAILURE);
        }
        return -1;
    }

    return counter;
}

double getMapDistance(rotaMap *a, rotaMap *b)
{
    if (a->index == b->index)
        return 0.0;
    return acos(calcVectorDotProduct(a->biom, b->biom, a->biomLen) / (a->biomVecLen * b->biomVecLen));
}