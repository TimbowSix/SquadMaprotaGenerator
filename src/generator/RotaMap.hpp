#pragma once

#include <iostream>
#include <string.h>
#include <vector>

#include "RotaLayer.hpp"
#include "RotaMode.hpp"

namespace rota
{

    class RotaMap
    {
    private:
        std::vector<RotaLayer *> layers;
        std::vector<float> biomValues;
        std::vector<float> mapWeights;
        std::vector<float> mapVoteWeights;
        std::vector<RotaMap *> neighbor;
        int neighborCount;
        int lockTime;
        int currentLockTime;
        std::vector<float> mapVoteWeightSum;
        float sigmoidValues[4]; // 0: mapVoteSlope, 1: mapVoteShift, 2: layerVoteSlope, 3: layerVoteShift
        // do we need cluster overlap ?
    public:
        RotaMap(char *name, std::vector<float> &biomValues);
        ~RotaMap();
        std::vector<RotaMode *> availableModes();
        std::vector<RotaLayer *> availableLayer();
        void newWeight(RotaMode &mode);

        void addLayer(RotaLayer *layer);

        void deceaseLockTime();
        void resetLockTime();
        void updateLockTime();

        void calcMapVoteWeights(RotaMode &mode);

        void calcMapWeights(RotaMode &mode, float params[WEIGHT_PARAMS_COUNT]);
    };
}