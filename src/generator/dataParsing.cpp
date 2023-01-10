#include "dataParsing.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <boost/json.hpp>
#include <regex>
#include <algorithm>

#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "utils.hpp"

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

    void parseLayers(std::string url, std::map<std::string, RotaMap*> *maps, std::map<std::string, RotaLayer*> *layers, std::map<std::string, RotaMode*> *modes){
        std::map<std::string, RotaLayer*> allLayers;
        getLayers(url, &allLayers); // get all layers from api
        std::regex pattern("^([a-zA-Z]+)_([a-zA-Z]+)_([a-zA-Z0-9]+)$");
        for(auto const& [key, layer] : (allLayers)){
            std::smatch match;
            std::string layerName = layer->getName();
            std::regex_match(layerName, match, pattern);
            if(match.empty()){
                continue; // layer doesn't match layer format, skip layer
            };
            std::string map = match[1];
            std::string mode = match[2];
            std::string version = match[3];

            if (maps->find(map) == maps->end()) {
                continue; // map doesn't exist in used maps, skip layer
            };
            if(modes->find(mode) == modes->end()){
                continue; // mode doesn't exist in used modes, skip layer
            };

            (*maps)[map]->addLayer(layer);
            (*layers)[key] = layer; //transfer layer to map of used layers
        }
    }

    void parseMaps(boost::json::object *config, std::map<std::string, RotaMap*> *maps){

        const std::filesystem::path configFile{"../../../data/bioms.json"};
        std::ifstream ifs(configFile);
        std::string data(std::istreambuf_iterator<char>{ifs}, {});
        boost::json::object biomValues = boost::json::parse(data).get_object();

        boost::json::array usedMapsRaw = config->at("maps").as_array();
        int locktime = config->at("biom_spacing").as_int64();
        for(int i=0; i<usedMapsRaw.size(); i++){
            std::string map = (std::string)usedMapsRaw[i].as_string();
            if(biomValues.find(map) == biomValues.end()){
                //no biom values for map, skip map
                std::cout << "WARNING: No Biom values saved for map '" << map << "'. Skipping map.\n";
                continue;
            }
            std::vector<float> biomVals;
            boost::json::array bv = biomValues.at(map).as_array();
            for(int i=0; i<bv.size(); i++){
                biomVals.push_back(bv[i].as_double());
            }
            (*maps)[map] = new RotaMap(map, biomVals, locktime);
        }
    }
} // namespace rota
