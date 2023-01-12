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
    this->biomValues = biomValues;
    this->lockTime = lockTime;
}

void RotaMap::addLayer(RotaLayer *layer) {
    this->layers.push_back(layer);
    this->modeToLayers[layer->getMode()].push_back(layer);
    if (this->availableLayers.find(layer->getMode()) ==
        this->availableLayers.end()) {
        // create if not found
        this->availableLayers[layer->getMode()] = 1;

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

void RotaMap::lock() { this->currentLockTime = this->lockTime; }

void RotaMap::lock(int locktime) { this->currentLockTime = locktime; }

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
    if (voteSum == 0) {
        throw std::runtime_error("div by 0, in calcMapVoteWeight");
        return;
    }
    float mean = 1 / voteSum * votes.size();
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

float RotaMap::calcMapWeight(RotaMode *mode) {
    float x = this->neighbor.size();
    assert(x >= 0);
    assert(this->mapVoteWeights[mode] > 0);
    float y = this->mapVoteWeights[mode] / this->mapVoteWeightSum[mode];
    return mode->weightParams[0] + mode->weightParams[1] * x +
           10 * mode->weightParams[2] * y + mode->weightParams[3] * pow(x, 2) +
           10 * mode->weightParams[4] * x * y +
           100 * mode->weightParams[5] * pow(y, 2);
}

void RotaMap::calcLayerVoteWeights() {
    float slope = this->sigmoidValues[2];
    float shift = this->sigmoidValues[3];
    for (RotaLayer *layer : this->layers) {
        layer->setVoteWeight(slope, shift);
    }
}

void RotaMap::calcNewMapVoteWeight(RotaMode *mode) {
    float oldWeight = this->mapVoteWeights[mode];
    this->calcMapVoteWeight(mode);
    this->mapVoteWeightSum[mode] += (this->mapVoteWeights[mode] - oldWeight);
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

std::vector<float> *RotaMap::getBiomValues() { return &this->biomValues; }
