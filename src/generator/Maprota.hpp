#pragma once

#include "RotaMode.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "RotaModePool.hpp"
#include <vector>
#include <boost/json.hpp>
#include <map>
#include "RotaTeam.hpp"

namespace rota
{
    class Maprota
    {
    private:
        boost::json::object *config;
        /**
         * @brief maps all available maps to their name
        */
        std::map<std::string, RotaMap*> maps;
        /**
         * @brief maps all available ModePools to their name
        */
        std::map<std::string, RotaModePool*> modePools;
        /**
         * @brief maps all modes to their name
        */
        std::map<std::string, RotaMode *> modes;
        /**
         * @brief maps all layers to their name
        */
        std::map<std::string, RotaLayer*> layers;
        /**
         * @brief maps all teams to their name
        */
        std::map<std::string, RotaTeam*> teams;
        /**
         * @brief all drawn modes in order
        */
        std::vector<RotaMode*> latestModes;
        /**
         * @brief all drawn layers
        */
        std::vector<RotaLayer*> rotation;
        /**
         * @brief all drawn maps in order
        */
        std::vector<RotaMap*> latestMaps;
        /**
         * @brief maps bluefor teams on containing layers
        */
       std::map<RotaTeam*, std::vector<RotaLayer*>> blueforTeams;
       /**
         * @brief maps opfor teams on containing layers
        */
       std::map<RotaTeam*, std::vector<RotaLayer*>> opforTeams;

    public:
        Maprota(boost::json::object *config);
        ~Maprota() {};

        /**
         * @brief Selects a random game mode based on the modes in the mode pools and the corresponding probabilities set in the configuration
         *
         * @param useLatestModes usage of latest modes for mode locking
         * @param customPool set pool to draw mode from
         * @returns pointer to choosen mode
        */
        RotaMode* chooseMode(bool useLatestModes, RotaModePool *customPool);

        RotaMap* chooseMap(RotaMode *mode);

        RotaLayer* chooseLayerFromMap(RotaMap *map, RotaMode *mode);

        /**
         * @brief Creates a new rotation based on the parameters set in the configuration.
         *
         * @returns rotation of layers
        */
        //std::vector<RotaLayer*> generateRota();
        void generateRota();
    };
} // namespace rota
