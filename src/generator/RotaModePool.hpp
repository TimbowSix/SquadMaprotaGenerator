/**
 * @file RotaModePool
 * @brief Object representation of multiple grouped RotaMode objects
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
*/

#pragma once

#include "RotaMode.hpp"
#include <string>
#include <vector>
#include <map>

namespace rota
{
    class RotaModePool
    {
    public:
        std::string name;
        float probability;
        std::map<std::string, RotaMode*> modes;
        RotaModePool(std::string name, float probability);
        ~RotaModePool();
        void addMode(RotaMode *mode);
    };
} // namespace rota
