
#include "RotaTeam.hpp"

using namespace rota;

RotaTeam::RotaTeam(std::string name){
    this->name = name;
}

std::string RotaTeam::getName(){
    return this->name;
}