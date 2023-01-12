#include "Maprota.hpp"

#include <vector>
#include <boost/json.hpp>
#include <string>
#include <numeric>

#include "RotaMode.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "RotaModePool.hpp"
#include "dataParsing.hpp"
#include "utils.hpp"

#include <iostream>

namespace rota
{
    Maprota::Maprota(boost::json::object *config){
        this->config = config;
        parseModes(this->config, &this->modePools, &this->modes);
        parseMaps(this->config, &this->maps); // setup all available maps

        std::string voteUrl = this->config->at("layer_vote_api_url").as_string().c_str();
        parseLayers(voteUrl, &this->maps, &this->layers, &this->modes); //request and parse layers

        std::string teamUrl = this->config->at("team_api_url").as_string().c_str();
        injectLayerInfo(teamUrl, &this->layers, &this->modes, &this->teams); // populate layers with data

        //remove maps without any layers
        for(auto it = this->maps.cbegin(); it != this->maps.cend();){
            if(it->second->getLayer()->size() == 0){
                std::cout << "WARNING: No layers available for map '" << it->second->getName() << "'" << std::endl;
                it = this->maps.erase(it);
            }else{
                it++;
            }
        }
        parseTeams(&this->layers, &this->blueforTeams, &this->opforTeams);
    }

    RotaMode* Maprota::chooseMode(bool useLatestModes=true, RotaModePool *customPool=nullptr){ //TODO test
        RotaModePool *pool;
        if(customPool != nullptr){
            pool = customPool;
        }else{
            std::vector<RotaModePool*> pools;
            std::vector<float> poolWeights;
            for(std::map<std::string,RotaModePool*>::iterator it = this->modePools.begin(); it != this->modePools.end(); ++it) {
                pools.push_back(it->second);
                poolWeights.push_back(it->second->probability);
            }
            pool = pools[weightedChoice(&poolWeights)];
        }

        if(useLatestModes && pool != this->modePools["main"]){
            int poolSpacing = this->config->at("mode_distribution").at("pool_spacing").as_int64();
            std::vector<RotaMode*> *latestModes = &this->latestModes;
            if(latestModes->size() > poolSpacing){
                *latestModes = std::vector<RotaMode*>(latestModes->begin()+latestModes->size()-poolSpacing, latestModes->end());
            }
            for(RotaMode* mode : (*latestModes)){
                if(this->modePools["main"]->modes.find(mode->name) == this->modePools["main"]->modes.end()){
                    pool = this->modePools["main"];
                }
            }
        }
        std::vector<RotaMode*> modes;
        std::vector<float> modeWeights;
        bool spaceMain = this->config->at("mode_distribution").at("space_main").as_bool();
        for(std::map<std::string,RotaMode*>::iterator it = pool->modes.begin(); it != pool->modes.end(); ++it) {
            if(spaceMain && pool == this->modePools["main"] && this->latestModes.size() > 0 && it->second == this->latestModes.back()){
                continue;
            }
            modes.push_back(it->second);
            modeWeights.push_back(it->second->probability);
        }
        normalize(&modeWeights, NULL); // normalize in case of excluded modes
        return modes[weightedChoice(&modeWeights)];
    }

    RotaMap* Maprota::chooseMap(RotaMode *mode){

        RotaMode *fallbackMode = chooseMode(true, this->modePools["main"]); //fallback mode of main pool if maps are unavailable for given mode
        std::vector<RotaMap*> fallbackAvailableMaps;
        std::vector<float> fallbackWeights;

        std::vector<RotaMap*> availableMaps;
        std::vector<float> weights;
        while(fallbackWeights.size() == 0){
            for(auto const& [key, map]: this->maps){
                if(!map->isLocked()){
                    if(map->hasLayersAvailable(mode)){ //get all maps containing mode and are unlocked
                        availableMaps.push_back(map);
                        weights.push_back(map->calcMapWeight(mode));
                    }
                    if(map->hasLayersAvailable(fallbackMode)){
                        fallbackAvailableMaps.push_back(map);
                        fallbackWeights.push_back(map->calcMapWeight(mode));
                    }
                }
            }
            this->decreaseMapLocktimes();
        }
        assert(fallbackWeights.size()>0);
        if(weights.size() == 0){
            this->modeBuffer = mode;
            normalize(&fallbackWeights, NULL);
            return fallbackAvailableMaps[weightedChoice(&fallbackWeights)];

        }else{
            normalize(&weights, NULL);
            return availableMaps[weightedChoice(&weights)];
        }
    }

    RotaLayer* Maprota::chooseLayerFromMap(RotaMap *map, RotaMode *mode){
        std::vector<float> weights;
        std::vector<RotaLayer*> layers;
        for(RotaLayer *layer: map->getModeToLayers()->at(mode)){
            if(!layer->isLocked()){
                //weights.push_back(layer->getVoteWeight());
                layers.push_back(layer);
            }
        }
        normalize(&weights, NULL);
        return layers[weightedChoice(&weights)];
    }

    void Maprota::decreaseMapLocktimes(){
        for(auto const& [key, map]: this->maps){
            map->decreaseLockTime();
        }
    }

    void Maprota::decreaseLayerLocktimes(){
        for(auto const& [key, layer]: this->layers){
            layer->decreaseLockTime();
        }
    }

    void Maprota::lockTeams(){
        int maxSameTeam = this->config->at("max_same_team").as_int64();
        if(maxSameTeam < 1) return;
        std::vector<RotaTeam*> teams1;
        std::vector<RotaTeam*> teams2;
        //get latest teams
        for(int i=this->rotation.size()-1-maxSameTeam; i<this->rotation.size(); i++){
            if(i%2==0){
                teams1.push_back(this->rotation[i]->getTeam(1));
                teams2.push_back(this->rotation[i]->getTeam(2));
            }else{
                teams1.push_back(this->rotation[i]->getTeam(2));
                teams2.push_back(this->rotation[i]->getTeam(1));
            }
        }
        RotaTeam *lock_blu = nullptr;
        RotaTeam *lock_op = nullptr;
        //check if latest teams in range maxSameTeam are all equal = team need to be locked
        if (all_of(teams1.begin(), teams1.end(), [&] (RotaTeam *i) {return i == teams1[0];})){
            //all are the same
            if (maxSameTeam % 2 == 0){
                lock_blu = teams1[0];
            }else{
                lock_op = teams1[0];
            }
        }
        if (all_of(teams2.begin(), teams2.end(), [&] (RotaTeam *i) {return i == teams2[0];})){
            //all are the same
            if (maxSameTeam % 2 == 0){
                lock_op = teams2[0];
            }else{
                lock_blu = teams2[0];
            }
        }

        // lock all layers containing relevant teams
        if(lock_blu != nullptr){
            for(RotaLayer *layer : this->blueforTeams[lock_blu]){
                layer->lock(1);
            }
        }
        if(lock_op != nullptr){
            for(RotaLayer *layer : this->opforTeams[lock_op]){
                layer->lock(1);
            }
        }
    }

    void Maprota::generateRota(){
        // add seedlayer
        if(config->at("seed_layer").as_int64() > 0){
            std::vector<RotaMap*> seedMaps;
            for(auto const& [key, map]: this->maps){
                std::map<RotaMode *, std::vector<RotaLayer *>> *modeLayers = map->getModeToLayers();
                if (modeLayers->find(this->modes["Seed"]) == modeLayers->end()) {
                    seedMaps.push_back(map);
                }
            }
            for(int i=0; i<this->config->at("seed_layer").as_int64(); i++){
                int choosenMap = choice(seedMaps.size());
                RotaMap *seedMap = seedMaps[choosenMap];
                seedMaps.erase(seedMaps.begin() + choosenMap); //remove choosen seed map to prevent doubles
                seedMap->lock(2);
                std::vector<RotaLayer*> seedLayers = seedMap->getModeToLayers()->at(this->modes["Seed"]);
                RotaLayer *chossenLayer = seedLayers[choice(seedLayers.size())];
            }
            this->latestModes.push_back(this->modes["Seed"]);
        }

        this->modeBuffer = nullptr;

        for(int i=0; i < this->config->at("number_of_layers").as_int64() - this->config->at("seed_layer").as_int64(); i++){
            RotaMode *mode;
            if(modeBuffer == nullptr){
                mode = chooseMode(true);
            }else{
                mode = modeBuffer;
                this->modeBuffer = nullptr;
            }
            RotaMap *map = chooseMap(mode);
            RotaLayer *layer = chooseLayerFromMap(map, mode);
            this->rotation.push_back(layer);
            this->latestMaps.push_back(map);
            this->latestModes.push_back(mode);
            this->decreaseLayerLocktimes();
            map->lock();
            layer->lock();
            this->lockTeams();
        }
    }

} // namespace rota

