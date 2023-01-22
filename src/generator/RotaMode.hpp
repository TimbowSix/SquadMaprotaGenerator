/**
 * @file RotaMode
 * @brief Object representation of a Squad Layer Mode
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */

#pragma once

#include "GlobalConfig.hpp"
#include <string>
#include <vector>

namespace rota {
class RotaMode {

  public:
    std::string name;
    float probability;
    RotaMode(std::string name, float probability);
    ~RotaMode(){};
};

} // namespace rota
