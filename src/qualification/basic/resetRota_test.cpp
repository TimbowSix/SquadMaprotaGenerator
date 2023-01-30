#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <Generator.hpp>
#include <GlobalConfig.hpp>
#include <RotaConfig.hpp>
#include <RotaLayer.hpp>

using namespace rota;

TEST(Reset_test, test_general_with_past_layers) {

    std::vector<RotaLayer *> rotaSource;
    std::vector<std::string> rota;
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    MemoryColonelState stateBefor;
    MemoryColonelState stateAfter;
    gen.getState(&stateBefor);

    for (RotaLayer *layer : *gen.getRota()) {
        rotaSource.push_back(layer);
        rota.push_back(layer->getName());
    }
    gen.reset(&rota);
    gen.getState(&stateAfter);

    ASSERT_EQ(stateBefor.mapState.size(), stateAfter.mapState.size());
    ASSERT_EQ(stateBefor.layerState.size(), stateAfter.layerState.size());
    ASSERT_EQ(stateBefor.genState.size(), stateAfter.genState.size());

    for (int i = 0; i < stateBefor.mapState.size(); i++) {
        ASSERT_EQ(stateBefor.mapState[i], stateAfter.mapState[i]) << i;
    }
    for (int i = 0; i < stateBefor.layerState.size(); i++) {
        ASSERT_EQ(stateBefor.layerState[i], stateAfter.layerState[i]) << i;
    }
    for (int i = 0; i < stateBefor.genState.size(); i++) {
        ASSERT_EQ(stateBefor.genState[i], stateAfter.genState[i]) << i;
    }
    ASSERT_EQ(stateBefor.lastTeam[0], stateAfter.lastTeam[0]);
    ASSERT_EQ(stateBefor.lastTeam[1], stateAfter.lastTeam[1]);
}