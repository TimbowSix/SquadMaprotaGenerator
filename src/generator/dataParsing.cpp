#pragma once
#include "dataParsing.hpp"

#include <boost/json.hpp>
#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include <string>
#include <vector>

namespace rota
{
    std::vector<RotaModePool> parseModes(boost::json::object *config){
        std::vector<RotaModePool> allPools;
        boost::json::value *pools = &config->at("mode_distribution").at("pools");
        for(auto currentPool = pools->as_object().begin(); currentPool != pools->as_object().end(); currentPool++){
            std::string poolName = currentPool->key_c_str();
            boost::json::object poolModes = currentPool->value().as_object();
            float poolProbability = config->at("mode_distribution").at("pool_distribution").at(poolName).as_double();
            RotaModePool pool(poolName, poolProbability);
            for(auto currentMode = poolModes.begin(); currentMode != poolModes.end(); currentMode++){
                std::string modeName = currentMode->key_c_str();
                float modeProbability = currentMode->value().as_double();
                RotaMode mode(modeName, modeProbability);
                pool.addMode(mode);
            }
            allPools.push_back(pool);
        }
        return allPools;
    }
} // namespace rota
