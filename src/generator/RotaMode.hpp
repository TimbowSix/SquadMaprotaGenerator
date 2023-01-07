#pragma once

#include "RotaConfig.hpp"
#include <string>

namespace rota
{
    class RotaMode
    {
    public:
        std::string name;
        float probability;
        //float weightParams[WEIGHT_PARAMS_COUNT];
        RotaMode(std::string name, float probability /*, float *weighParams*/);
        ~RotaMode();
    };

} // namespace rota
