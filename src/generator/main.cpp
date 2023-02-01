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

using namespace rota;

int main(void) {
    std::cout << "Version " << ROTA_VERSION_MAJOR << "." << ROTA_VERSION_MINOR
              << std::endl;

    std::string path = std::string(CONFIG_PATH) + "config.json";
    rota::RotaConfig conf(path);
    rota::Generator gen(&conf);
    std::cout << "Seed: " << gen.getSeed() << std::endl << std::endl;

    // gen data stuff
    std::ofstream file;
    file.open(std::to_string(time(NULL)) + ".dat");
    std::vector<rota::RotaLayer *> ges;

    for (int j = 0; j < 500; j++) {
        std::cout << j << std::endl;

        gen.setRandomMapWeights();
        for (int i = 0; i < 1000; i++) {
            gen.generateRota();
            for (RotaLayer *layer : *gen.getRota()) {
                // std::cout << layer->getName() << std::endl;
                ges.push_back(layer);
            }
            gen.reset();
        }
        // std::map<RotaMap *, float> expDist;
        std::map<RotaMap *, float> genDist;
        int sum = 0;

        for (RotaLayer *layer : ges) {
            if (layer->getMode()->name.compare("RAAS") == 0) {
                if (genDist.count(layer->getMap()) == 1) {
                    genDist.at(layer->getMap())++;
                } else {
                    genDist[layer->getMap()] = 1;
                }
                sum++;
            }
        }

        for (auto const &x : genDist) {
            // std::cout << x.first->getName() << " " << x.second << std::endl;
            genDist[x.first] = x.second / (float)sum;
        }

        for (auto const &x : genDist) {
            file << x.first->getNeighbor()->size() << ";"
                 << x.first->getMapWeight(gen.getModes()->at("RAAS")) << ";"
                 << x.second << "\n";
        }
        ges.clear();
    }
    file.close();

    /*for (rota::RotaLayer *layer : *gen.getRota()) {
        std::cout << layer->getName() << std::endl;
    }*/

    return 0;
}
