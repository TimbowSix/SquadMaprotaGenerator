#include "dataParsing.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <boost/json.hpp>
#include <regex>

#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"

#include <iostream>

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
    }

    void parseMaps(
        boost::json::object *config,
        std::map<std::string, RotaMap*> *maps,
        std::map<std::string, RotaLayer*> *layers,
        std::map<std::string, RotaMode*> *modes){
            
        const std::filesystem::path configFile{"../../../data/bioms.json"};
        std::ifstream ifs(configFile);
        std::string data(std::istreambuf_iterator<char>{ifs}, {});
        boost::json::object biomValues = boost::json::parse(data).get_object();

        boost::json::array usedMaps = config->at("maps").as_array();
        //std::cout << config->at("maps") << "\n";
        std::regex pattern("^([a-zA-Z]+)_([a-zA-Z]+)_([a-zA-Z0-9]+)$");
        for(auto const& [key, value] : (*layers)){
            std::smatch match;
            //std::cout << key << std::endl;
            std::string layer = value->getName();
            std::regex_match(layer, match, pattern);
            if(match.empty()){
                continue
            };

            std::string map = match[1];
            std::string mode = match[2];
            std::string version = match[3];


            std::cout << map << " " << mode << " " << version << std::endl;

        }
    }
} // namespace rota
