#pragma once

#include <string>

namespace rota {

class RotaTeam {
  private:
    std::string name;

  public:
    RotaTeam(std::string name);
    ~RotaTeam(){};

    std::string getName();
};

} // namespace rota