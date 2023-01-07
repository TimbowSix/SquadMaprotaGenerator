#pragma once

#include "RotaMode.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "RotaModePool.hpp"
#include <vector>
#include <boost/json.hpp>

namespace rota
{
    class Maprota
    {
    private:
        boost::json::object *config;
        std::vector<RotaMap> maps;
        std::vector<RotaModePool> modePools;
        std::vector<RotaMode> latestModes;
        std::vector<RotaLayer> rotation;
        std::vector<RotaMap> latestMaps;
        void initializeModes();
    public:
        Maprota(boost::json::object *config);
        ~Maprota();

        /**
         * @param useLatestModes usage of latest modes for mode locking
         * @param customPool set pool to draw mode from
        */
        RotaMode chooseMode(bool useLatestModes, RotaModePool *customPool);
    };
} // namespace rota
