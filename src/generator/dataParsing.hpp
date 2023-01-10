#pragma once

#include <boost/json.hpp>
#include "RotaModePool.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"

namespace rota
{
    /**
     * @brief parsing all used modes from config.json
     *
     * @param config pointer to config.json object
     * @param allPools empty map of modePools
     * @param allModes empty map of modes
    */
    void parseModes(boost::json::object *config, std::map<std::string, RotaModePool *> *allPools, std::map<std::string, RotaMode *> *allModes);

    /**
     * @brief requesting layers from url and parsing used layers
     *
     * @param url Layer API Url
     * @param maps map of used RotaMaps
     * @param layers empty map of used layers
     * @param modes map of used modes
    */
    void parseLayers(std::string url, std::map<std::string, RotaMap*> *maps, std::map<std::string, RotaLayer*> *layers, std::map<std::string, RotaMode*> *modes);

    /**
     * @brief parsing all used maps from config, populate them with layers and biom values
     *
     * @param config config.json object
     * @param maps empty map of RotaMaps
     * @param layers map of layers
     * @param modes map of modes to include
     *
    */
    void parseMaps(
        boost::json::object *config,
        std::map<std::string, RotaMap*> *maps,
        std::map<std::string, RotaLayer*> *layers,
        std::map<std::string, RotaMode*> *modes);
} // namespace rota
