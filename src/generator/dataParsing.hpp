#pragma once

#include <boost/json.hpp>
#include "RotaModePool.hpp"

namespace rota
{
    /**
     * parsing all used modes from config.json
     * @param config pointer to config.json object
    */
    std::vector<RotaModePool> parseModes(boost::json::object *config);
} // namespace rota
