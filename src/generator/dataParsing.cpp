#include "dataParsing.hpp"

#include <boost/json.hpp>
#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include <string>
#include <vector>
#include <map>

namespace rota
{
    void parseModes(boost::json::object *config, std::map<std::string, RotaModePool *> *allPools, std::map<std::string, RotaMode *> *allModes){
        boost::json::value *pools = &config->at("mode_distribution").at("pools");
        for(auto currentPool = pools->as_object().begin(); currentPool != pools->as_object().end(); currentPool++){
            std::string poolName = currentPool->key_c_str();
            boost::json::object poolModes = currentPool->value().as_object();
            float poolProbability = config->at("mode_distribution").at("pool_distribution").at(poolName).as_double();
            RotaModePool *pool = new RotaModePool(poolName, poolProbability);
            for(auto currentMode = poolModes.begin(); currentMode != poolModes.end(); currentMode++){
                std::string modeName = currentMode->key_c_str();
                float modeProbability = currentMode->value().as_double();
                RotaMode *mode = new RotaMode(modeName, modeProbability);
                pool->addMode(mode);
                (*allModes)[modeName] = mode;
            }
            (*allPools)[poolName] = pool;
        }
        //return allPools;
    }
} // namespace rota
