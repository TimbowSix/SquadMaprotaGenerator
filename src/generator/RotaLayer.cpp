#include <assert.h>
#include "RotaLayer.hpp"

using namespace rota;

RotaLayer::RotaLayer(std::string name, float votes){
    this->name = name;
    this->votes = votes;
}




//getter
std::string RotaLayer::getName(){
    return this->name;
}

float RotaLayer::getVotes(){
    return this->votes;
}

RotaTeam* RotaLayer::getTeam(int index){
    assert(index < 2);
    return this->teams[index];
}

RotaMode* RotaLayer::getMode(){
    return this->mode;
}


//setter
void RotaLayer::setTeam(RotaTeam *team, int index){
    assert(index < 2);
    this->teams[index] = team;
}
void RotaLayer::setMode(RotaMode *mode){
    this->mode = mode;
}