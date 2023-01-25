#include <boost/json.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "Generator.hpp"
#include "GlobalConfig.hpp"
#include "RotaConfig.hpp"
#include "RotaLayer.hpp"
#include "utils.hpp"

int main(void) {
    std::cout << "Version " << ROTA_VERSION_MAJOR << "." << ROTA_VERSION_MINOR
              << std::endl;

    std::string path = std::string(CONFIG_PATH) + "config.json";
    rota::RotaConfig conf(path);
    rota::Generator gen(&conf);

    std::cout << "Seed: " << gen.getSeed() << std::endl;
    gen.generateRota();

    return 0;
}
