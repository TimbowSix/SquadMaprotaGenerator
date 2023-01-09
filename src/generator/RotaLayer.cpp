#include "RotaLayer.hpp"
#include "utils.hpp"
#include <assert.h>

using namespace rota;

RotaLayer::RotaLayer(std::string name, float votes) {
    this->name = name;
    this->votes = votes;
}
// getter
std::string RotaLayer::getName() { return this->name; }

float RotaLayer::getVotes() { return this->votes; }

RotaTeam *RotaLayer::getTeam(int index) {
    assert(index < 2);
    return this->teams[index];
}

RotaMode *RotaLayer::getMode() { return this->mode; }

bool RotaLayer::isLocked() { return currLockTime == 0; }

// setter
void RotaLayer::setTeam(RotaTeam *team, int index) {
    assert(index < 2);
    this->teams[index] = team;
}
void RotaLayer::setMode(RotaMode *mode) { this->mode = mode; }

void RotaLayer::setVoteWeight(float slope, float shift) {
    this->voteWeight = sigmoid(this->votes, slope, shift);
}
