#include "Generator.hpp"

#include <boost/json.hpp>
#include <numeric>
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
    parseMaps(this->config, &this->mapsByName); // setup all available maps

    std::string voteUrl = this->config->get_layer_vote_api_url();
    parseLayers(voteUrl, &this->mapsByName, &this->layers,
                &this->modes); // request and parse layers

    std::string teamUrl = this->config->get_team_api_url();
    injectLayerInfo(teamUrl, &this->layers, &this->modes,
                    &this->teams); // populate layers with data

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
    }
    setNeighbour(&this->maps, this->config->get_min_biom_distance());
}

RotaMode *
Generator::chooseMode(bool useLatestModes = true,
                      RotaModePool *customPool = nullptr) { // TODO test
    RotaModePool *pool;
    if (customPool != nullptr) {
        pool = customPool;
    } else {
        std::vector<RotaModePool *> pools;
        std::vector<float> poolWeights;
        for (std::map<std::string, RotaModePool *>::iterator it =
                 this->modePools.begin();
             it != this->modePools.end(); ++it) {
            pools.push_back(it->second);
            poolWeights.push_back(it->second->probability);
        }
        pool = pools[weightedChoice(&poolWeights)];
    }

    if (useLatestModes && pool != this->modePools["main"]) {
        int poolSpacing = this->config->get_pool_spacing();
        std::vector<RotaMode *> *latestModes = &this->latestModes;
        if (latestModes->size() > poolSpacing) {
            *latestModes = std::vector<RotaMode *>(
                latestModes->begin() + latestModes->size() - poolSpacing,
                latestModes->end());
        }
        for (RotaMode *mode : (*latestModes)) {
            if (this->modePools["main"]->modes.find(mode->name) ==
                this->modePools["main"]->modes.end()) {
                pool = this->modePools["main"];
            }
        }
    }
    std::vector<RotaMode *> modes;
    std::vector<float> modeWeights;
    bool spaceMain = this->config->get_space_main();
    for (std::map<std::string, RotaMode *>::iterator it = pool->modes.begin();
         it != pool->modes.end(); ++it) {
        if (spaceMain && pool == this->modePools["main"] &&
            this->latestModes.size() > 0 &&
            it->second == this->latestModes.back()) {
            continue;
        }
        modes.push_back(it->second);
        modeWeights.push_back(it->second->probability);
    }
    normalize(&modeWeights, NULL); // normalize in case of excluded modes
    return modes[weightedChoice(&modeWeights)];
}

RotaMap *Generator::chooseMap(RotaMode *mode) { // TODO Test?

    RotaMode *fallbackMode = chooseMode(
        true, this->modePools["main"]); // fallback mode of main pool if maps
                                        // are unavailable for given mode
    std::vector<RotaMap *> fallbackAvailableMaps;
    std::vector<float> fallbackWeights;

    std::vector<RotaMap *> availableMaps;
    std::vector<float> weights;
    while (fallbackWeights.size() == 0) {
        for (RotaMap *map : this->maps) {
            if (!map->isLocked()) {
                if (map->hasLayersAvailable(mode)) { // get all maps containing
                                                     // mode and are unlocked
                    availableMaps.push_back(map);
                    weights.push_back(map->getMapVoteWeight(mode));
                }
                if (map->hasLayersAvailable(fallbackMode)) {
                    fallbackAvailableMaps.push_back(map);
                    fallbackWeights.push_back(map->getMapVoteWeight(mode));
                }
            }
        }
        this->decreaseMapLocktimes();
    }
    assert(fallbackWeights.size() > 0);
    if (weights.size() == 0) {
        this->modeBuffer = mode;
        normalize(&fallbackWeights, NULL);
        return fallbackAvailableMaps[weightedChoice(&fallbackWeights)];

    } else {
        normalize(&weights, NULL);
        return availableMaps[weightedChoice(&weights)];
    }
}

RotaLayer *Generator::chooseLayerFromMap(RotaMap *map,
                                         RotaMode *mode) { // Todo Test
    std::vector<float> weights;
    std::vector<RotaLayer *> layers;
    for (RotaLayer *layer : map->getModeToLayers()->at(mode)) {
        if (!layer->isLocked()) {
            // weights.push_back(layer->getVoteWeight());
            layers.push_back(layer);
        }
    }
    normalize(&weights, NULL);
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
    std::vector<RotaTeam *> teams1;
    std::vector<RotaTeam *> teams2;
    // get latest teams
    for (int i = this->rotation.size() - 1 - maxSameTeam;
         i < this->rotation.size(); i++) {
        if (i % 2 == 0) {
            teams1.push_back(this->rotation[i]->getTeam(1));
            teams2.push_back(this->rotation[i]->getTeam(2));
        } else {
            teams1.push_back(this->rotation[i]->getTeam(2));
            teams2.push_back(this->rotation[i]->getTeam(1));
        }
    }
    RotaTeam *lock_blu = nullptr;
    RotaTeam *lock_op = nullptr;
    // check if latest teams in range maxSameTeam are all equal = team need to
    // be locked
    if (all_of(teams1.begin(), teams1.end(),
               [&](RotaTeam *i) { return i == teams1[0]; })) {
        // all are the same
        if (maxSameTeam % 2 == 0) {
            lock_blu = teams1[0];
        } else {
            lock_op = teams1[0];
        }
    }
    if (all_of(teams2.begin(), teams2.end(),
               [&](RotaTeam *i) { return i == teams2[0]; })) {
        // all are the same
        if (maxSameTeam % 2 == 0) {
            lock_op = teams2[0];
        } else {
            lock_blu = teams2[0];
        }
    }

    // lock all layers containing relevant teams
    if (lock_blu != nullptr) {
        for (RotaLayer *layer : this->blueforTeams[lock_blu]) {
            layer->lock(1);
        }
    }
    if (lock_op != nullptr) {
        for (RotaLayer *layer : this->opforTeams[lock_op]) {
            layer->lock(1);
        }
    }
}

void Generator::generateRota() {
    // add seedlayer
    if (config->get_seed_layer() > 0) {
        std::vector<RotaMap *> seedMaps;
        for (RotaMap *map : this->maps) {
            std::map<RotaMode *, std::vector<RotaLayer *>> *modeLayers =
                map->getModeToLayers();
            if (modeLayers->find(this->modes["Seed"]) == modeLayers->end()) {
                seedMaps.push_back(map);
            }
        }
        for (int i = 0; i < this->config->get_seed_layer(); i++) {
            int choosenMap = choice(seedMaps.size());
            RotaMap *seedMap = seedMaps[choosenMap];
            seedMaps.erase(
                seedMaps.begin() +
                choosenMap); // remove choosen seed map to prevent doubles
            seedMap->lock(2);
            std::vector<RotaLayer *> seedLayers =
                seedMap->getModeToLayers()->at(this->modes["Seed"]);
            RotaLayer *chossenLayer = seedLayers[choice(seedLayers.size())];
        }
        this->latestModes.push_back(this->modes["Seed"]);
    }

    this->modeBuffer = nullptr;

    for (int i = 0; i < this->config->get_number_of_layers() -
                            this->config->get_seed_layer();
         i++) {
        RotaMode *mode;
        if (modeBuffer == nullptr) {
            mode = chooseMode(true);
        } else {
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

void Generator::reset() {
    std::vector<RotaLayer *> pastLayers;
    this->reset(&pastLayers); // call reset with empty past layers
}
void Generator::reset(std::vector<RotaLayer *> *pastLayers) {
    this->rotation.clear();
    this->latestMaps.clear();
    this->latestModes.clear();
    this->modeBuffer = nullptr;
    for (RotaMap *map : this->maps) {
        map->unlock();
    }
    for (auto const &[key, layer] : this->layers) {
        layer->unlock();
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
            latestModes.push_back(pastLayers->at(i)->getMode());
        }
    }
}
void Generator::reset(std::vector<std::string> *latestLayers) {
    std::vector<RotaLayer *> pastLayers;
    for (std::string layerName : (*latestLayers)) {
        RotaLayer *layer = layers[layerName]; // TODO handle layer not found
        pastLayers.push_back(layer);
    }
    reset(&pastLayers);
}

} // namespace rota
