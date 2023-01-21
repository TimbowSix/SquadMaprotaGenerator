#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "RotaLayer.hpp"
#include "RotaMap.hpp"
#include "utils.hpp"

using namespace rota;

RotaMap::RotaMap(std::string name, std::vector<float> biomValues,
                 int lockTime) {
    this->name = name;
    this->biomValues.resize(biomValues.size());
    for (int i = 0; i < biomValues.size(); i++) {
        this->biomValues(i) = biomValues[i];
    }
    this->lockTime = lockTime;
}

void RotaMap::addLayer(RotaLayer *layer) {
    this->layers.push_back(layer);
    this->modeToLayers[layer->getMode()].push_back(layer);
    if (this->availableLayers.find(layer->getMode()) ==
        this->availableLayers.end()) {
        // create if mode not found
        this->availableLayers[layer->getMode()] = 1;
        // init value for weights for this mode
        this->mapVoteWeights[layer->getMode()] = 1;
    } else {
        this->availableLayers[layer->getMode()]++;
    }
}

void RotaMap::decreaseLockTime() {
    this->currentLockTime--;
    if (this->currentLockTime < 0) {
        this->currentLockTime = 0;
    }
}

void RotaMap::lock() { this->lock(this->lockTime, true); }

void RotaMap::lock(bool lockNeighbors) {
    this->lock(this->lockTime, lockNeighbors);
}

void RotaMap::lock(int locktime) { this->lock(locktime, false); }

void RotaMap::lock(int locktime, bool lockNeighbors) {
    assert(locktime > 0);
    if (this->currentLockTime <
        locktime) { // do not overwrite existing locktimes
        this->currentLockTime = locktime;

        if (lockNeighbors) {
            for (RotaMap *map : this->neighbor) {
                map->lock(false);
            }
        }
    }
}

void RotaMap::overwriteLock(int locktime) {
    assert(locktime > 0);
    this->currentLockTime = locktime;
}

void RotaMap::unlock() { this->currentLockTime = 0; }

void RotaMap::calcMapVoteWeight(RotaMode *mode) {
    if (this->layers.size() == 0) {
        std::cout << "No Layer in map " << this->name
                  << " could not calculate map vote weights" << std::endl;
        return;
    }

    std::vector<float> votes;
    std::vector<float> weights;

    float voteSum = 0.0;

    for (auto layer : this->modeToLayers[mode]) {
        if (!layer->isLocked()) {
            voteSum += layer->getVotes();
            votes.push_back(layer->getVotes());
        }
    }
    if (votes.size() == 0) {
        throw std::runtime_error("div by 0, in calcMapVoteWeight");
        return;
    }
    float mean = 1.0 / votes.size() * voteSum;
    float sum = 0;
    for (float val : votes) {
        float temp = exp(-std::pow(mean - val, 2));
        weights.push_back(temp);
        sum += temp;
    }
    normalize(&weights, &sum);

    float weightSum = 0;
    for (int i = 0; i < weights.size(); i++) {
        weights[i] *= votes[i];
        weightSum += weights[i];
    }

    this->mapVoteWeights[mode] =
        sigmoid(weightSum, this->sigmoidValues[0], this->sigmoidValues[1]);

    votes.clear();
    weights.clear();
}

void RotaMap::calcAllMapVoteWeights() {
    for (auto const &[key, val] : this->modeToLayers) {
        this->calcMapVoteWeight(key);
    }
}

void RotaMap::calcLayerVoteWeights() {
    float slope = this->sigmoidValues[2];
    float shift = this->sigmoidValues[3];
    for (RotaLayer *layer : this->layers) {
        layer->setVoteWeight(slope, shift);
    }
}

bool RotaMap::hasLayersAvailable(RotaMode *mode) {
    if (this->hasMode(mode)) {
        return this->availableLayers[mode] > 0;
    } else {
        return false;
    }
}

bool RotaMap::hasMode(RotaMode *mode) {
    return this->availableLayers.find(mode) == this->availableLayers.end();
}

// getter & setter
void RotaMap::setLockTime(int lockTime) { this->lockTime = lockTime; }
std::vector<RotaLayer *> *RotaMap::getLayer() { return &this->layers; }

std::string RotaMap::getName() { return this->name; }
std::map<RotaMode *, std::vector<RotaLayer *>> *RotaMap::getModeToLayers() {
    return &this->modeToLayers;
}
bool RotaMap::isLocked() { return this->currentLockTime > 0; }

void RotaMap::increaseAvailableLayers(RotaMode *mode) {
    this->availableLayers[mode]++;
}
void RotaMap::decreaseAvailableLayers(RotaMode *mode) {
    this->availableLayers[mode]--;
}

boost::numeric::ublas::vector<float> *RotaMap::getBiomValues() {
    return &this->biomValues;
}

void RotaMap::addNeighbour(RotaMap *map) { this->neighbor.push_back(map); }

void RotaMap::setSigmoidValues(float mapVoteSlope, float mapVoteShift,
                               float layerVoteSlope, float layerVoteShift) {
    this->sigmoidValues[0] = mapVoteSlope;
    this->sigmoidValues[1] = mapVoteShift;
    this->sigmoidValues[2] = layerVoteSlope;
    this->sigmoidValues[3] = layerVoteShift;
}

float RotaMap::getMapVoteWeight(RotaMode *mode) {
    return this->mapVoteWeights[mode];
}
void RotaMap::setMapVoteWeight(RotaMode *mode, float weight) {
    this->mapVoteWeights[mode] = weight;
}
