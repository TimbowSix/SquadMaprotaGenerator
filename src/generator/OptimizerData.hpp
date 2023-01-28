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

typedef struct OptData OptData;

struct OptData {
    std::map<rota::RotaMap *, std::map<rota::RotaMode *, float>> data;
};
