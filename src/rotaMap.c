#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "rotaMap.h"
#include "utils.h"

void newMap(rotaMap *map, int maxMapCount, int maxLayerCount, int maxModeCount)
{
    map->layerCount = 0;
    map->modeCount = maxModeCount;
    map->currentLayersLockedCount = 0;
    map->currentLockTime = 0;

    map->neighbour = malloc(maxMapCount * sizeof(rotaMap *));
    map->layers = malloc(maxLayerCount * sizeof(rotaLayer *));
    map->modes = malloc(maxModeCount * sizeof(rotaMode *));

    for (int i = 0; i < maxModeCount; i++)
    {
        map->modes[i] = NULL;
    }

    map->mapWeights = malloc(maxModeCount * sizeof(rotaMode *));
    map->mapVoteWeights = malloc(maxModeCount * sizeof(rotaMode *));
    map->mapVoteWeightSum = malloc(maxModeCount * sizeof(rotaMode *));
    map->sigmoidValues = malloc(4 * sizeof(double));

    map->lockLayer = lockLayer;
    map->newWeight = newWeight;
    map->resetLayerLockTime = resetLayerLockTime;
    map->decreaseLayerLockTime = decreaseLayerLockTime;
    map->addLayer = addLayer;
    map->decreaseLockTime = decreaseLockTime;
    map->setLockTime = setLockTime;
    map->calcMapVoteWeight = calcMapVoteWeight;
    map->calcLayerVoteWeight = calcLayerVoteWeight;
    map->calcMapWeight = calcMapWeight;
}

void delMap(rotaMap *map)
{
    free(map->name);
    free(map->neighbour);
    free(map->layers);
    free(map->modes);
    free(map->mapVoteWeights);
    free(map->mapWeights);
    free(map->mapVoteWeightSum);
    free(map->sigmoidValues);
    free(map);
}

void lockLayer(rotaLayer *layer, rotaMap *self)
{
    if (layer->currentLockTime == 0)
    {
        self->currentLayersLockedCount++;
    }
    layer->currentLockTime = layer->lockTime;
}

void newWeight(rotaMode *mode, rotaMap *self)
{
    double oldWeight = self->mapWeights[mode->index];
    self->calcMapVoteWeight(mode, self);
    self->mapVoteWeightSum[mode->index] = oldWeight + self->mapVoteWeights[mode->index];
}

void resetLayerLockTime(rotaMap *self)
{
    for (int i = 0; i < self->layerCount; i++)
    {
        self->layers[i]->currentLockTime = 0;
    }
    self->currentLayersLockedCount = 0;
}

void decreaseLayerLockTime(rotaMap *self)
{
    for (int i = 0; i < self->layerCount; i++)
    {
        if (self->layers[i]->currentLockTime > 0)
        {
            self->layers[i]->currentLockTime--;
            if (self->layers[i]->currentLockTime == 0)
            {
                self->currentLayersLockedCount--;
                assert(self->currentLayersLockedCount >= 0);
            }
        }
    }
}

void addLayer(rotaLayer *layer, rotaMap *self)
{
    self->layers[self->layerCount] = layer;
    self->layerCount++;

    // mode not found -> add
    self->modes[layer->mode->index] = layer->mode;
}

void decreaseLockTime(rotaMap *self)
{
    if (self->currentLockTime > 0)
    {
        self->currentLockTime--;
    }
}

void setLockTime(rotaMap *self)
{
    self->currentLockTime = self->lockTime;
}

void calcMapVoteWeight(rotaMode *mode, rotaMap *self)
{
    double slope = self->sigmoidValues[0];
    double shift = self->sigmoidValues[1];

    // check if map has layers
    if (self->layerCount == 0)
    {
        printf("No layers added to map %s, could not calculate map vote weights!", self->name);
        return;
    }

    // calc all mapVoteWeights
    // TODO what is faster malloc and free mem or loop a second time thought all layers ?
    double *votes = malloc(self->layerCount * sizeof(double));
    double *weights = malloc(self->layerCount * sizeof(double));

    if (votes == NULL || weights == NULL)
    {
        printf("Error cannot allocate memory");
        exit(EXIT_FAILURE);
    }

    int voteCount = 0;

    double voteSum = 0;

    for (int j = 0; j < self->layerCount; j++)
    {
        // check if layer is not locked
        if (self->layers[j]->currentLockTime == 0)
        {
            voteSum += self->layers[j]->votes;
            votes[voteCount] = self->layers[j]->votes;
            voteCount++;
        }
    }

    // check if mode has layers
    if (voteCount > 0)
    {
        double mean = 1 / voteSum * voteCount;
        double sum = 0;

        for (int j = 0; j < voteCount; j++)
        {
            double temp = exp(-pow(mean - votes[j], 2));
            weights[j] = temp;
            sum += temp;
        }

        normalize(weights, voteCount, &sum);

        double weightsSum = 0;
        for (int j = 0; j < voteCount; j++)
        {
            weights[j] *= votes[j];
            weightsSum += weights[j];
        }

        self->mapVoteWeights[mode->index] = sigmoid(weightsSum, slope, shift);
    }

    free(votes);
    free(weights);
}

void calcAllMapVoteWeight(rotaMap *self)
{
    for (int i = 0; i < self->modeCount; i++)
    {
        if (self->modes[i] != NULL)
        {
            calcMapVoteWeight(self->modes[i], self);
        }
    }
}

void calcLayerVoteWeight(rotaMap *self)
{
    double slope = self->sigmoidValues[2];
    double shift = self->sigmoidValues[3];

    for (int i = 0; i < self->layerCount; i++)
    {
        self->layers[i]->voteWeight = sigmoid(self->layers[i]->votes, slope, shift);
    }
}

double calcMapWeight(rotaMode *mode, rotaMap *self)
{
    assert(WEIGHT_PARAMS_COUNT == 6);

    double x = self->neighbourCount - 1;
    assert(x >= 0);
    assert(self->mapVoteWeightSum[mode->index] > 0);
    double y = self->mapVoteWeights[mode->index] / self->mapVoteWeightSum[mode->index];
    return mode->weightParams[0] + mode->weightParams[1] * x + 10 * mode->weightParams[2] * y + mode->weightParams[3] * pow(x, 2) + 10 * mode->weightParams[4] * x * y + 100 * mode->weightParams[5] * pow(y, 2);
}