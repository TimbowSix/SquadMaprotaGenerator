#include "Generator.hpp"

#include <boost/json.hpp>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <RotaOptimizer.hpp>

#include "RotaConfig.hpp"
#include "RotaLayer.hpp"
#include "RotaMap.hpp"
#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include "dataParsing.hpp"
#include "utils.hpp"

#include <iostream>

namespace rota {
Generator::Generator(RotaConfig *config) {
    this->config = config;
    this->modePools = (*config->get_pools());
    this->modes = (*config->get_modes());
    parseMaps(this->config, &this->mapsByName,
              &this->availableLayerMaps); // setup all available maps

    std::string voteUrl = this->config->get_layer_vote_api_url();
    std::string teamUrl = this->config->get_team_api_url();
    parseLayers(voteUrl, teamUrl, &this->mapsByName, &this->layers,
                &this->modes, &this->teams); // request and parse layers

    injectLayerInfo(teamUrl, &this->layers, &this->modes, &this->teams);
    // populate layers with data

    // remove maps without any layers
    for (auto it = this->mapsByName.cbegin(); it != this->mapsByName.cend();) {
        if (it->second->getLayer()->size() == 0) {
            std::cout << "WARNING: No layers available for map '"
                      << it->second->getName() << "'" << std::endl;
            it = this->mapsByName.erase(it);
        } else {
            it++;
        }
    }
    parseTeams(&this->layers, &this->blueforTeams, &this->opforTeams);
    for (auto const &[key, map] : this->mapsByName) {
        this->maps.push_back(map);

        for (RotaMode *m : *(map->getModes())) {
            // init modeToMapList
            this->modeToMapList[m->name].push_back(map);
            // init availableMaps
            if (this->availableLayerMaps.find(m) ==
                this->availableLayerMaps.end()) {
                this->availableLayerMaps[m] = 1;
            } else {
                this->availableLayerMaps[m]++;
            }
        }
    }

    setNeighbour(&this->maps, this->config->get_min_biom_distance());

    // set reference from layer to ther maps, sets lockTime, sets voteWeight
    // inits teamToLayerList
    for (RotaMap *map : this->maps) {
        for (RotaLayer *layer : *(map->getLayer())) {
            layer->setMap(map);
            layer->setLockTime(this->config->get_layer_locktime());
            layer->setVoteWeight(this->config->get_layervote_slope(),
                                 this->config->get_layervote_shift());
        }
    }

    // precalculate default Rota Mode Pool weights and sum
    for (std::map<std::string, RotaModePool *>::iterator it =
             this->modePools.begin();
         it != this->modePools.end(); ++it) {
        this->defaultModePools.push_back(it->second);
        this->defaultPoolWeights.push_back(it->second->probability);
        // precalculate modes weights
        float tempSum = 0.0;
        for (auto const &x : it->second->modes) {
            this->modeWeights[it->second].push_back(x.second->probability);
            this->poolToModeList[it->second].push_back(x.second);
            tempSum += x.second->probability;
        }
        normalize(&this->modeWeights[it->second], &tempSum);
    }
    this->lastNonMainMode = 0;
    this->nextMainModeIndex = 0;

    // printMapNeighbor(&this->maps);
    this->seed = time(NULL);
}

RotaMode *Generator::chooseMode(RotaModePool *customPool = nullptr,
                                bool ignoreModeBuff = false,
                                int depth = 0) { // TODO test

    if (depth > 5) {
        printMemColonel(&this->maps);
        throw std::runtime_error("Error in choose Mode max depth reached");
        return nullptr;
    }

    RotaModePool *pool;
    // choose Pool
    if (customPool != nullptr) {
        pool = customPool;
    } else {
        // check mode buffer
        if (!(ignoreModeBuff) && this->modeBuffer.size() > 0) {
            // check if mode is conform with pool spacing
            if (this->modeBuffer.back()->isMainMode ||
                this->lastNonMainMode >= this->config->get_pool_spacing()) {
                // check if mode has available maps
                // TODO  force buffed mode ?
                if (this->mapsAvailable(this->modeBuffer.back())) {
                    RotaMode *mode = this->modeBuffer.back();
                    this->modeBuffer.pop_back();
                    return mode;
                }
            }
        }
        pool =
            this->defaultModePools[weightedChoice(&this->defaultPoolWeights)];
    }

    if (pool != this->modePools["main"]) {
        // check if non main mode has enough space to last one
        if (this->config->get_pool_spacing() > this->lastNonMainMode) {
            // not enough space change to main mode
            pool = this->modePools["main"];
        }
    }

    if (pool == this->modePools["main"]) {
        this->lastNonMainMode++;
    } else {
        this->lastNonMainMode = 0;
    }

    RotaMode *ret;

    if (this->config->get_space_main() && pool == this->modePools["main"]) {
        // TODO Überarbeitung gleichverteilung
        ret = this->poolToModeList[pool][this->nextMainModeIndex];
    } else {
        ret = this->poolToModeList[pool]
                                  [weightedChoice(&this->modeWeights[pool])];
    }
    // check if mode has available maps
    if (this->mapsAvailable(ret)) {
        if (this->config->get_space_main() && pool == this->modePools["main"]) {
            this->nextMainModeIndex = (this->nextMainModeIndex + 1) %
                                      this->modePools["main"]->modes.size();
        }
        return ret;
    }
    // no map available for this map -> mode buffer -> new mode
    return chooseMode(customPool, true, ++depth);
}

RotaMap *Generator::chooseMap(RotaMode *mode) { // TODO Test?

    if (!this->mapsAvailable(mode)) {
        throw std::runtime_error("no maps available for mode: " + mode->name);
        return nullptr;
    }

    std::vector<RotaMap *> availableMaps;
    std::vector<float> weights;
    float tempSum = 0.0;

    for (RotaMap *map : this->modeToMapList[mode->name]) {
        if (!map->isLocked() && map->hasLayersAvailable(mode)) {
            availableMaps.push_back(map);
            weights.push_back(map->getMapVoteWeight(mode));
            tempSum += map->getMapVoteWeight(mode);
        }
    }

    if (weights.size() == 0) {
        printMemColonel(&this->maps);
        throw std::runtime_error("error in chooseMaps, no maps for mode " +
                                 mode->name);
        return nullptr;
    }

    normalize(&weights, &tempSum);
    return availableMaps[weightedChoice(&weights)];
}

RotaLayer *Generator::chooseLayerFromMap(RotaMap *map,
                                         RotaMode *mode) { // Todo Test
    std::vector<float> weights;
    std::vector<RotaLayer *> layers;
    float sum = 0.0;
    for (RotaLayer *layer : map->getModeToLayers()->at(mode)) {
        if (!layer->isLocked()) {
            sum += layer->getVoteWeight();
            weights.push_back(layer->getVoteWeight());
            layers.push_back(layer);
        }
    }
    if (weights.size() <= 0) {
        throw std::runtime_error("No Layer available for mode: " + mode->name +
                                 " choosen map: " + map->getName());
    }

    normalize(&weights, &sum);
    return layers[weightedChoice(&weights)];
}

void Generator::decreaseMapLocktimes() {
    for (RotaMap *map : this->maps) {
        map->decreaseLockTime();
    }
}

void Generator::decreaseLayerLocktimes() {
    for (auto const &[key, layer] : this->layers) {
        layer->decreaseLockTime();
    }
}

void Generator::lockTeams() { // TODO test?
    int maxSameTeam = this->config->get_max_same_team();
    if (maxSameTeam < 1)
        return;

    if (this->sameTeamCounter[0] >= maxSameTeam) {
        // lock layers with team 1
        for (RotaLayer *layer :
             this->blueforTeams[this->teamHistory[0].back()]) {
            layer->lock(1);
        }
    }

    if (this->sameTeamCounter[1] >= maxSameTeam) {
        // lock layers with team 1

        for (RotaLayer *layer : this->opforTeams[this->teamHistory[1].back()]) {
            layer->lock(1);
        }
    }
}

void Generator::generateRota() {
    // set seed
    srand(this->seed);
    // add seedlayer
    if (config->get_seed_layer() > 0) {
        std::vector<RotaMap *> seedMaps(this->modeToMapList["Seed"]);

        for (int i = 0; i < this->config->get_seed_layer(); i++) {
            int choosenMap = choice(seedMaps.size());
            RotaMap *seedMap = seedMaps[choosenMap];
            seedMaps.erase(
                seedMaps.begin() +
                choosenMap); // remove choosen seed map to prevent doubles
            seedMap->lock(2);

            std::vector<RotaLayer *> seedLayers =
                seedMap->getModeToLayers()->at(this->modes["Seed"]);
            RotaLayer *chosenLayer = seedLayers[choice(seedLayers.size())];
            rotation.push_back(chosenLayer);
        }
        this->ModesHistory.push_back(this->modes["Seed"]);
    }

    this->modeBuffer.clear();

    int currTeamIndex[2] = {0, 1};
    RotaTeam *tempTeam[2];

    for (int i = 0; i < this->config->get_number_of_layers() -
                            this->config->get_seed_layer();
         i++) {
        RotaMode *mode = chooseMode(nullptr, true);
        RotaMap *map = chooseMap(mode);
        RotaLayer *layer = chooseLayerFromMap(map, mode);
        this->rotation.push_back(layer);
        this->MapsHistory.push_back(map);
        this->ModesHistory.push_back(mode);

        for (int i = 0; i < 2; i++) {
            this->teamHistory[i].push_back(layer->getTeam(currTeamIndex[i]));
            currTeamIndex[i] = (currTeamIndex[i] + 1) % 2;
            if (layer->getTeam(currTeamIndex[i]) != tempTeam[i]) {
                this->sameTeamCounter[i] = 0;
                tempTeam[i] = layer->getTeam(currTeamIndex[i]);
            } else {
                this->sameTeamCounter[i]++;
            }
        }

        this->decreaseMapLocktimes();
        this->decreaseLayerLocktimes();
        map->lock();
        layer->lock();
        this->lockTeams();

        // std::cout << layer->getName() << std::endl;
        // printMemColonel(&this->maps);
        // std::cout << std::endl;
    }
}

void Generator::reset() {
    std::vector<RotaLayer *> pastLayers;
    time_t seed = time(NULL);
    this->reset(&pastLayers, seed); // call reset with empty past layers
}
void Generator::reset(std::vector<RotaLayer *> *pastLayers, time_t seed) {
    // this->seed = seed;
    this->lastNonMainMode = 0;
    this->rotation.clear();
    this->MapsHistory.clear();
    this->ModesHistory.clear();
    this->modeBuffer.clear();

    for (auto const &[key, layer] : this->layers) {
        layer->unlock();
    }
    for (RotaMap *map : this->maps) {
        map->unlock();
    }

    // set previous layers
    int pastLen = pastLayers->size();
    if (pastLen == 0)
        return;
    for (int i = 0; i < pastLen; i++) {
        if (pastLen - i <
            config->get_biom_spacing() +
                1) { // lock maps within biom spacing and their neighbors
            pastLayers->at(i)->getMap()->lock(
                config->get_biom_spacing() - pastLen - 1 - i, true);
        }
        pastLayers->at(i)->lock(config->get_layer_locktime() - pastLen - 1 -
                                i); // lock layers
        if (pastLen - i <
            config->get_pool_spacing() + 1) { // add modes within the range of
                                              // pool spacing to latest modes
            ModesHistory.push_back(pastLayers->at(i)->getMode());
        }
    }
}
void Generator::reset(std::vector<std::string> *latestLayers) {
    std::vector<RotaLayer *> pastLayers;
    for (std::string layerName : (*latestLayers)) {
        RotaLayer *layer = layers.at(layerName);
        pastLayers.push_back(layer);
    }
    time_t seed = time(NULL);
    reset(&pastLayers, seed);
}

bool Generator::mapsAvailable(RotaMode *mode) {
    return (this->availableLayerMaps[mode] > 0);
}

time_t Generator::getSeed() { return this->seed; }

} // namespace rota
