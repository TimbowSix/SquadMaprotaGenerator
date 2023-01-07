#pragma once

#include "RotaMode.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "RotaModePool.hpp"
#include <vector>
#include <boost/json.hpp>
#include <map>

namespace rota
{
    class Maprota
    {
    private:
        boost::json::object *config;
        std::map<std::string, RotaMap*> maps;
        std::map<std::string, RotaModePool*> modePools;
        std::map<std::string, RotaMode *> modes;
        std::vector<RotaMode*> latestModes;
        std::vector<RotaLayer*> rotation;
        std::vector<RotaMap*> latestMaps;
    public:
        Maprota(boost::json::object *config);
        ~Maprota() {};

        /**
         * Selects a random game mode based on the modes in the mode pools and the corresponding probabilities set in the configuration
         * @param useLatestModes usage of latest modes for mode locking
         * @param customPool set pool to draw mode from
        */
        RotaMode* chooseMode(bool useLatestModes, RotaModePool *customPool);
    };
} // namespace rota
