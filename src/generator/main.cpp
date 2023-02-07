#include <boost/json.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "Generator.hpp"
#include "GlobalConfig.hpp"
#include "OptimizerData.hpp"
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

    /*gen.generateRota();

    for (rota::RotaLayer *layer : *gen.getRota()) {
        std::cout << layer->getName() << std::endl;
    }*/

    // gen data stuff
    std::ofstream file;
    file.open(std::to_string(time(NULL)) + ".dat");
    std::vector<rota::RotaLayer *> ges;
    std::map<RotaMap *, float> genDist;
    std::map<RotaMap *, float>::iterator it;

    std::string modeName = "Destruction";

    for (int j = 0; j < 1000; j++) {
        std::cout << j << std::endl;

        gen.setRandomMapWeights(gen.getModes()->at(modeName));
        for (int i = 0; i < 1; i++) {
            gen.generateRota();
            for (RotaLayer *layer : *gen.getRota()) {
                ges.push_back(layer);
            }
            gen.reset();
        }

        int sum = 0;

        for (RotaLayer *layer : ges) {
            if (layer->getMode()->name.compare(modeName) == 0) {
                it = genDist.find(layer->getMap());
                // file << layer->getName() << "\n";
                if (it != genDist.end()) {
                    genDist[layer->getMap()] += 1.0;
                } else {
                    genDist[layer->getMap()] = 1.0;
                }
                sum++;
            }
        }

        for (auto const &x : genDist) {
            // file << x.first->getName() << " " << x.second << std::endl;
            genDist[x.first] = x.second / sum;
        }
        // file << sum << "\n";

        for (auto const &x : genDist) {
            file << x.first->getNeighbor()->size() << ";"
                 << x.first->getMapWeight(gen.getModes()->at(modeName)) << ";"
                 << x.second << "\n";
        }
        // file << "\n";
        ges.clear();
        genDist.clear();
    }
    file.close();

    return 0;
}
