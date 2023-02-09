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
    conf.set_number_of_layers(100000);
    rota::Generator gen(&conf);
    std::cout << "Seed: " << gen.getSeed() << std::endl << std::endl;

    // gen data stuff
    std::vector<rota::RotaLayer *> ges;
    std::map<RotaMode *, std::map<RotaMap *, float>> genDist;

    std::map<RotaMode *, int> sum;

    for (auto const &x : *gen.getModes()) {
        if (x.second->probability > 0 && x.second->modePool != nullptr) {
            sum[x.second] = 0;
        }
    }

    for (int j = 0; j < 1000; j++) {
        std::cout << j << std::endl;

        for (auto const &x : *gen.getModes()) {
            if (x.second->probability > 0 && x.second->modePool != nullptr) {
                gen.setRandomMapWeights(x.second);
            }
        }
        for (int i = 0; i < 1; i++) {
            gen.generateRota();
            for (RotaLayer *layer : *gen.getRota()) {
                ges.push_back(layer);
            }
            gen.reset();
        }

        for (RotaLayer *layer : ges) {

            if (layer->getMode()->probability > 0 &&
                layer->getMode()->modePool != nullptr) {

                // file << layer->getName() << "\n";
                if (genDist[layer->getMode()].count(layer->getMap())) {
                    genDist[layer->getMode()][layer->getMap()] += 1.0;
                } else {
                    genDist[layer->getMode()][layer->getMap()] = 1.0;
                }
                sum.at(layer->getMode())++;
            }
        }

        for (auto const &x : genDist) {
            for (auto const &y : genDist[x.first]) {
                genDist[x.first][y.first] = y.second / sum[x.first];
            }
        }
        // file << sum << "\n";

        for (auto const &x : genDist) {

            std::ofstream file;
            file.open(std::string(x.first->name + ".dat"));
            file << "neighbor; weight; distribution\n";

            for (auto const &y : genDist[x.first]) {
                file << y.first->getNeighbor()->size() << ";"
                     << y.first->getMapWeight(x.first) << ";" << y.second
                     << "\n";
            }
            file.close();
        }
        // file << "\n";
        ges.clear();
        for (auto const &x : genDist) {
            genDist[x.first].clear();
        }
        genDist.clear();
    }

    return 0;
}
