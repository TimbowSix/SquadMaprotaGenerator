#pragma once

#include <boost/json.hpp>
#include "RotaModePool.hpp"

namespace rota
{
    /**
     * parsing all used modes from config.json
     * @param config pointer to config.json object
     * @param allPools empty map of modePools
     * @param allModes empty map of modes
    */
    void parseModes(boost::json::object *config, std::map<std::string, RotaModePool *> *allPools, std::map<std::string, RotaMode *> *allModes);
} // namespace rota
