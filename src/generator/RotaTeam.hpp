#pragma once

#include <string>

namespace rota {

class RotaTeam {
  private:
    std::string name;

  public:
    RotaTeam(std::string name);

    std::string getName();
};

} // namespace rota