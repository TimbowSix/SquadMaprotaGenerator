#include "RotaMode.hpp"
#include <string>
#include <vector>

namespace rota {
RotaMode::RotaMode(std::string name, float probability) {
    this->name = name;
    this->probability = probability;
}

} // namespace rota
