/**
 * @file RotaMode
 * @brief Object representation of a Squad Layer Mode
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */

#pragma once

#include "RotaConfig.hpp"
#include <string>
#include <vector>

namespace rota {
class RotaMode {

  public:
    std::string name;
    float probability;
    float weightParams[WEIGHT_PARAMS_COUNT];
    RotaMode(std::string name, float probability , std::vector<float> weightParams);
    ~RotaMode() {};
};

} // namespace rota
