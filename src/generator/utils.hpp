#pragma once
#include <vector>

#include "RotaLayer.hpp"

namespace rota
{
    int weightedChoice(std::vector<float> *weights);

    RotaLayer *getLayers(std::string url);
} // namespace rota
