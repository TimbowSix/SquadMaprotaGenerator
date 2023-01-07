#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include <string>
#include <vector>

namespace rota
{
    RotaModePool::RotaModePool(std::string name, float probability) {
        this->name = name;
        this->probability = probability;
    };

    RotaModePool::~RotaModePool(){};

    void RotaModePool::addMode(RotaMode *mode){
        this->modes[mode->name] = mode;
    };

} // namespace rota
