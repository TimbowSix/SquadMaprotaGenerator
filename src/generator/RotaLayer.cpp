#include "RotaLayer.hpp"
#include "RotaMap.hpp"
#include "utils.hpp"
#include <assert.h>

using namespace rota;

RotaLayer::RotaLayer(std::string name, float votes) {
    this->name = name;
    this->votes = votes;
    this->currLockTime = 0;
}

void RotaLayer::lock() {
    // this->currLockTime = this->lockTime;
    this->lock(this->lockTime);
}

void RotaLayer::lock(unsigned int time) {
    assert(time > 0);
    if (this->currLockTime < time) { // do not overwrite existing locktimes
        this->currLockTime = time;
        this->map->decreaseAvailableLayers(this->mode);
    }
}

void RotaLayer::unlock() {
    if (this->isLocked()) {
        this->map->increaseAvailableLayers(this->mode);
    }
    this->currLockTime = 0;
}

void RotaLayer::decreaseLockTime() {
    if (this->isLocked()) {
        this->currLockTime--;
        if (!this->isLocked()) {
            this->map->increaseAvailableLayers(this->mode);
        }
    }
}

// getter
std::string RotaLayer::getName() { return this->name; }

float RotaLayer::getVotes() { return this->votes; }

RotaTeam *RotaLayer::getTeam(int index) {
    assert(index < 2);
    return this->teams[index];
}

RotaMode *RotaLayer::getMode() { return this->mode; }

bool RotaLayer::isLocked() { return currLockTime != 0; }

// setter
void RotaLayer::setTeam(RotaTeam *team, int index) {
    assert(index < 2);
    this->teams[index] = team;
}
void RotaLayer::setMode(RotaMode *mode) { this->mode = mode; }

void RotaLayer::setVoteWeight(float slope, float shift) {
    this->voteWeight = sigmoid(this->votes, slope, shift);
}

RotaMap *RotaLayer::getMap() { return this->map; }

void RotaLayer::setMap(RotaMap *map) { this->map = map; }

void RotaLayer::setLockTime(int lockTime) { this->lockTime = lockTime; }

float RotaLayer::getVoteWeight() { return this->voteWeight; }

int RotaLayer::getLockTime() { return this->currLockTime; }