/**
 * @file OptimizerData
 * @brief Struct do parse Data between Optimizer and Generator
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */

#pragma once
#include <map>
#include <vector>

#include "RotaMap.hpp"
#include "RotaMode.hpp"

typedef struct OptDataIn OptDataIn;
typedef struct OptDataOut OptDataOut;

struct OptDataIn {
    std::vector<float> mapDist;
    std::map<int, std::vector<int>> clusters;
};

struct OptDataOut {
    std::vector<float> mapWeights;
};