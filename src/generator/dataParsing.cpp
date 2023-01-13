#include "dataParsing.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <boost/json.hpp>
#include <regex>
#include <algorithm>

#include "Config.hpp"
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
        (*allModes)["Seed"] = new RotaMode("Seed", 1.0); //add Seeding mode
    }

    void parseLayers(std::string url, std::map<std::string, RotaMap*> *maps, std::map<std::string, RotaLayer*> *layers, std::map<std::string, RotaMode*> *modes){
        Config cfg(std::string(CONFIG_PATH)+"/config.json");
        //std::map<std::string, RotaLayer*> allLayers;
        std::vector<RotaLayer*> allLayers;
        getLayers(url, &allLayers); // get all layers from api
        std::regex pattern("^([a-zA-Z]+)_([a-zA-Z]+)_([a-zA-Z0-9]+)$");
        for(RotaLayer* layer : allLayers){
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
            (*layers)[layer->getName()] = layer; //transfer layer to map of used layers
        }
    }

    void parseMaps(Config *config, std::map<std::string, RotaMap*> *maps){

        const std::filesystem::path configFile{std::string(CONFIG_PATH)+"/data/bioms.json"};
        std::ifstream ifs(configFile);
        std::string data(std::istreambuf_iterator<char>{ifs}, {});
        boost::json::object biomValues = boost::json::parse(data).get_object();

        int test = config->get_layer_locktime();
        std::vector<std::string> *usedMaps = config->get_maps();
        int locktime = config->get_biom_spacing();
        for(std::string map : (*usedMaps)){
            //std::string map = usedMapsRaw[i];
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

    void parseTeams(
        std::map<std::string, RotaLayer*> *layers,
        std::map<RotaTeam*, std::vector<RotaLayer*>> *blueforTeams,
        std::map<RotaTeam*, std::vector<RotaLayer*>> *opforTeams)
    {
        for(auto const& [key, layer]: (*layers)){

            RotaTeam *teamOne = layer->getTeam(0);
            if (blueforTeams->find(teamOne) == blueforTeams->end()){
                (*blueforTeams)[teamOne] = std::vector<RotaLayer*> {layer};
            }else{
                (*blueforTeams)[teamOne].push_back(layer);
            }

            RotaTeam *teamTwo = layer->getTeam(1);
            if (opforTeams->find(teamTwo) == opforTeams->end()){
                (*opforTeams)[teamTwo] = std::vector<RotaLayer*> {layer};
            }else{
                (*opforTeams)[teamTwo].push_back(layer);
            }
        }
    }
} // namespace rota
