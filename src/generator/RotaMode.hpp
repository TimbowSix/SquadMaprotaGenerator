#pragma once

#include "RotaConfig.hpp"

namespace rota
{
    class RotaMode
    {
    private:
        int index;
        char *name;
        float probability;
        float weightParams[WEIGHT_PARAMS_COUNT];

    public:
        RotaMode(int index, char *name, float probability, float *weighParams);
        ~RotaMode();
    };

} // namespace rota
