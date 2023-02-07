#include "Generator.hpp"

#include <boost/json.hpp>
#include <immintrin.h>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

#include <RotaOptimizer.hpp>

#include "OptimizerData.hpp"
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
    int i = 0;
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

    // inti ids
    for (auto const &x : this->modeToMapList) {
        int i = 0;
        for (RotaMap *map : x.second) {
            map->setId(i, this->modes.at(x.first));
            i++;
        }
    }

    setNeighbour(&this->maps, this->config->get_min_biom_distance());

    for (RotaMap *map : this->maps) {
        // set reference from layer to ther maps, sets lockTime, sets voteWeight
        // inits teamToLayerList
        for (RotaLayer *layer : *(map->getLayer())) {
            layer->setMap(map);
            layer->setLockTime(this->config->get_layer_locktime());
            layer->setVoteWeight(this->config->get_layervote_slope(),
                                 this->config->get_layervote_shift());
        }

        map->calcAllMapVoteWeights();
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
    this->sameTeamCounter[0] = 0;
    this->sameTeamCounter[1] = 0;

    this->generateSeed();
}

Generator::~Generator() {
    for (auto &x : this->teams) {
        delete x.second;
    }
    for (int i = 0; i < this->maps.size(); i++) {
        delete maps[i];
    }
    for (auto &x : this->modePools) {
        delete x.second;
    }
    for (auto &x : this->modes) {
        delete x.second;
    }
    for (auto &x : this->layers) {
        delete x.second;
    }
};

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
        pool = this->defaultModePools[weightedChoice(&this->defaultPoolWeights,
                                                     this->rng)];
    }

    if (pool != this->modePools["main"]) {
        // check if non main mode has enough space to last one
        if (this->config->get_pool_spacing() > this->lastNonMainMode) {
            if (this->modeBuffer.size() == 0) {
                // add mode of not possible Pool to Mode Buffer
                this->modeBuffer.push_back(this->poolToModeList.at(pool).at(
                    weightedChoice(&this->modeWeights.at(pool), this->rng)));
            }
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
        ret = this->poolToModeList[pool][weightedChoice(
            &this->modeWeights[pool], this->rng)];
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
    this->modeBuffer.push_back(ret);
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

    for (RotaMap *map : this->modeToMapList.at(mode->name)) {
        if (!map->isLocked() && map->hasLayersAvailable(mode)) {
            availableMaps.push_back(map);
            weights.push_back(map->getMapWeight(mode));
            tempSum += map->getMapWeight(mode);
        }
    }

    if (weights.size() == 0) {
        // printMemColonel(&this->maps);
        throw std::runtime_error("error in chooseMaps, no maps for mode " +
                                 mode->name + " seed " +
                                 std::to_string(this->seed));
        return nullptr;
    }

    normalize(&weights, &tempSum);
    return availableMaps.at(weightedChoice(&weights, this->rng));
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
    return layers[weightedChoice(&weights, this->rng)];
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

    if (this->sameTeamCounter[this->currTeamIndex[0]] >= maxSameTeam) {
        // lock layers with team 1
        for (RotaLayer *layer :
             this->blueforTeams[this->teamHistory[this->currTeamIndex[0]]
                                    .back()]) {
            layer->lock(1);
        }
    }

    if (this->sameTeamCounter[this->currTeamIndex[1]] >= maxSameTeam) {
        // lock layers with team 1

        for (RotaLayer *layer :
             this->opforTeams[this->teamHistory[this->currTeamIndex[1]]
                                  .back()]) {
            layer->lock(1);
        }
    }
}

void Generator::generateRota() {
    // set seed
    this->rng.seed(this->seed);
    // add seedlayer
    if (config->get_seed_layer() > 0) {
        std::vector<RotaMap *> seedMaps(this->modeToMapList["Seed"]);

        for (int i = 0; i < this->config->get_seed_layer(); i++) {
            int choosenMap = choice(seedMaps.size(), this->rng);
            RotaMap *seedMap = seedMaps[choosenMap];
            seedMaps.erase(
                seedMaps.begin() +
                choosenMap); // remove choosen seed map to prevent doubles
            seedMap->lock(2);

            std::vector<RotaLayer *> seedLayers =
                seedMap->getModeToLayers()->at(this->modes["Seed"]);
            RotaLayer *chosenLayer =
                seedLayers[choice(seedLayers.size(), this->rng)];
            rotation.push_back(chosenLayer);
        }
        this->ModesHistory.push_back(this->modes["Seed"]);
    }

    this->modeBuffer.clear();

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
            this->teamHistory[i].push_back(
                layer->getTeam(this->currTeamIndex[i]));

            if (layer->getTeam(this->currTeamIndex[i]) != this->lastTeam[i]) {
                this->sameTeamCounter[i] = 1;
                this->lastTeam[i] = layer->getTeam(this->currTeamIndex[i]);
            } else {
                this->sameTeamCounter[i]++;
            }
            this->currTeamIndex[i] = (this->currTeamIndex[i] + 1) % 2;
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

void Generator::generateOffer(std::vector<RotaLayer *> *out, int count) {
    for (int i = 0; i < count; i++) {
        RotaMode *mode = chooseMode(nullptr, true);
        RotaMap *map = chooseMap(mode);
        RotaLayer *layer = chooseLayerFromMap(map, mode);
        out->push_back(layer);
        layer->lock();
    }
}

void Generator::reset() {
    std::vector<RotaLayer *> pastLayers;
    this->reset(&pastLayers); // call reset with empty past layers
}
void Generator::reset(std::vector<RotaLayer *> *pastLayers) {
    this->generateSeed();
    this->lastNonMainMode = 0;
    this->sameTeamCounter[0] = 0;
    this->sameTeamCounter[1] = 0;
    this->currTeamIndex[0] = 0;
    this->currTeamIndex[1] = 1;
    this->rotation.clear();
    this->MapsHistory.clear();
    this->ModesHistory.clear();
    this->modeBuffer.clear();
    this->teamHistory[0].clear();
    this->teamHistory[1].clear();

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
        // lock maps
        if (pastLen - i <=
            this->config->get_biom_spacing()) { // lock maps within biom spacing
                                                // and their neighbors
            pastLayers->at(i)->getMap()->lock(
                this->config->get_biom_spacing() - (pastLen - i) + 1, true);
        }
        // lock layers
        if (pastLen - i <= this->config->get_layer_locktime()) {
            pastLayers->at(i)->lock(this->config->get_layer_locktime() -
                                    (pastLen - i) + 1);
        }

        // update mode space counter
        if (pastLayers->at(i)->getMode()->isMainMode) {
            this->lastNonMainMode++;
        } else {
            this->lastNonMainMode = 0;
        }

        // update team counter
        if (pastLayers->at(i)->getMode()->modePool != nullptr) {
            // mode is not seed aka has no pool
            for (int j = 0; j < 2; j++) {
                if (pastLayers->at(i)->getTeam(this->currTeamIndex[j]) !=
                    this->lastTeam[j]) {
                    this->sameTeamCounter[j] = 1;
                    this->lastTeam[j] =
                        pastLayers->at(i)->getTeam(this->currTeamIndex[j]);
                } else {
                    this->sameTeamCounter[j]++;
                }
                this->currTeamIndex[j] = (this->currTeamIndex[j] + 1) % 2;
            }
        }
    }
}
void Generator::reset(std::vector<std::string> *latestLayers) {
    std::vector<RotaLayer *> pastLayers;
    for (std::string layerName : (*latestLayers)) {
        RotaLayer *layer = layers.at(layerName);
        pastLayers.push_back(layer);
    }
    reset(&pastLayers);
}

bool Generator::mapsAvailable(RotaMode *mode) {
    return (this->availableLayerMaps[mode] > 0);
}

void Generator::setMapWeights(OptDataOut *data, RotaMode *mode) {
    int i = 0;
    for (RotaMap *m : this->maps) {
        if (m->hasMode(mode)) {
            assert(m->getId(mode) == i);

            for (RotaMap *map : this->maps) {
                if (map->getId(mode) == i) {
                    map->setMapWeight(mode, data->mapWeights.at(i));
                    break;
                }
            }
            i++;
        }
    }
}

void Generator::packOptData(OptDataIn *data, RotaMode *mode) {
    int i = 0;

    for (RotaMap *m : this->maps) {
        if (m->hasMode(mode)) {
            data->mapDist.push_back(m->getMapVoteWeight(mode));
            for (RotaMap *map : *m->getNeighbor()) {
                if (map->hasMode(mode)) {
                    data->clusters[i].push_back(map->getId(mode));
                }
            }
            i++;
        }
    }
    normalize(&data->mapDist);
}

u_int32_t Generator::getSeed() { return this->seed; }

std::vector<RotaLayer *> *Generator::getRota() { return &this->rotation; }

void Generator::getState(MemoryColonelState *state) {
    for (RotaMap *map : this->maps) {
        state->mapState.push_back(map->getCurrLockTime());
        for (auto const &x : *map->getAvailableLayers())
            state->layerLockState.push_back(x.second);
    }
    for (auto const &x : this->layers) {
        state->layerState.push_back(x.second->getLockTime());
    }
    for (auto const &x : this->availableLayerMaps) {
        state->layerMapLockState.push_back(x.second);
    }

    state->genState.push_back(this->lastNonMainMode);
    state->genState.push_back(this->sameTeamCounter[0]);
    state->genState.push_back(this->sameTeamCounter[1]);
    state->genState.push_back(this->currTeamIndex[0]);
    state->genState.push_back(this->currTeamIndex[1]);

    state->lastTeam[0] = this->lastTeam[0];
    state->lastTeam[1] = this->lastTeam[1];
}

void Generator::setRandomMapWeights(RotaMode *mode) {
    std::uniform_real_distribution<float> dist(0, 1);
    for (RotaMap *map : this->maps) {
        if (map->hasMode(mode)) {
            map->setMapWeight(mode, dist(this->rng));
        }
    }
}

void Generator::generateSeed() {
    std::random_device os_seed; // seed used by the mersenne-twister-engine
    this->seed = os_seed();
}

std::map<std::string, RotaMode *> *Generator::getModes() {
    return &this->modes;
}

} // namespace rota
