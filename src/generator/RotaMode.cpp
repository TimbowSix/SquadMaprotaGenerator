#include "RotaMode.hpp"
#include <string>
#include <vector>

namespace rota {
RotaMode::RotaMode(std::string name, float probability, bool isMainMode,
                   RotaModePool *modePool) {
    this->name = name;
    this->probability = probability;
    this->isMainMode = isMainMode;
    this->modePool = modePool;
}

} // namespace rota
