#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <Generator.hpp>
#include <GlobalConfig.hpp>
#include <RotaConfig.hpp>
#include <RotaLayer.hpp>

using namespace rota;

TEST(Test, test_numberRotaLayer) {

    std::vector<RotaLayer *> rotaSource;
    std::vector<std::string> rota;
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_rotas(1);
    conf.set_number_of_layers(100);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();
    for (RotaLayer *layer : *gen.getRota()) {
        rotaSource.push_back(layer);
        rota.push_back(layer->getName());
    }
    gen.reset(&rota);
}