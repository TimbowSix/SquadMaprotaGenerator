#include "Maprota.hpp"
#include "RotaMode.hpp"
#include "RotaMap.hpp"
#include "RotaLayer.hpp"
#include "RotaModePool.hpp"
#include <vector>
#include <boost/json.hpp>
#include "dataParsing.hpp"
#include "utils.hpp"

#include <numeric>

namespace rota
{
    Maprota::Maprota(boost::json::object *config){
        this->config = config;
        parseModes(this->config, &this->modePools, &this->modes);
    }

    RotaMode* Maprota::chooseMode(bool useLatestModes=true, RotaModePool *customPool=nullptr){
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

} // namespace rota

