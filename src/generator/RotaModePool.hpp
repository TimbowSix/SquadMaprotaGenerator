#pragma once

#include "RotaMode.hpp"
#include <string>
#include <vector>

namespace rota
{
    class RotaModePool
    {
    public:
        std::string name;
        float probability;
        std::vector<RotaMode> modes;
        RotaModePool(std::string name, float probability);
        ~RotaModePool();
        void addMode(RotaMode mode);
    };
} // namespace rota
