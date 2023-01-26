#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <Generator.hpp>
#include <GlobalConfig.hpp>
#include <RotaConfig.hpp>
#include <RotaLayer.hpp>

using namespace rota;

class Rota_Fixture : public ::testing::Test {

  protected:
    Generator *gen;
    RotaConfig *conf;

    void SetUp() override {
        conf = new RotaConfig(std::string(CONFIG_PATH) + "config.json");

        conf->set_number_of_rotas(1);
        conf->set_number_of_layers(100000);
        conf->set_seed_layer(1);
        conf->set_space_main(true);

        gen = new Generator(conf);
        gen->reset();
        gen->generateRota();
    }

    void TearDown() override {
        delete gen;
        delete conf;
    }
};

TEST_F(Rota_Fixture, test_numberRotaLayer) {
    std::vector<rota::RotaLayer *> *rota = this->gen->getRota();
    ASSERT_EQ(rota->size(), 100000);
}

TEST_F(Rota_Fixture, test_SpaceMain) {
    std::vector<rota::RotaLayer *> *rota = this->gen->getRota();

    RotaLayer *temp = nullptr;

    for (RotaLayer *layer : *rota) {
        if (temp != nullptr && layer->getMode()->isMainMode &&
            temp->getMode()->isMainMode) {
            ASSERT_TRUE(layer->getMode() != temp->getMode());
        }
        temp = layer;
    }
}