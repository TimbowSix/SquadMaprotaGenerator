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
     * @brief parsing all used maps from config, populate them with layers and biom values
     *
     * @param
    */
    void parseMaps(boost::json::object *config, std::map<std::string, RotaMap*> *maps, std::map<std::string, RotaLayer*> *layers);
} // namespace rota
