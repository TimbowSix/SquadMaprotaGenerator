#include "RotaMode.hpp"
#include <string>
#include <vector>

namespace rota
{
RotaMode::RotaMode(std::string name, float probability, std::vector<float> weightParams){
    this->name = name;
    this->probability = probability;
    for(int i=0; i<weightParams.size(); i++){
        this->weightParams[i] = weightParams[i];
    }
}


} // namespace rota
