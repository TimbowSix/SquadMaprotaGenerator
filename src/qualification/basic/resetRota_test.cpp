#include <cstddef>
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
    std::vector<RotaLayer *> rotaSource;
    std::vector<std::string> rota;

    void SetUp() override {
        conf = new RotaConfig(std::string(CONFIG_PATH) + "config.json");

        conf->set_number_of_rotas(1);
        conf->set_number_of_layers(1000);
        conf->set_seed_layer(1);
        conf->set_space_main(true);

        gen = new Generator(conf);
        gen->reset();
        gen->generateRota();
        for (RotaLayer *layer : *gen->getRota()) {
            rotaSource.push_back(layer);
            rota.push_back(layer->getName());
        }
        gen->reset(&rota);
        gen->generateRota();
        for (RotaLayer *layer : *gen->getRota()) {
            rotaSource.push_back(layer);
        }
    }

    void TearDown() override {
        rotaSource.clear();
        rota.clear();
        delete gen;
        delete conf;
    }
};

TEST_F(Rota_Fixture, test_numberRotaLayer) {
    std::vector<rota::RotaLayer *> *rota = this->gen->getRota();
    ASSERT_EQ(rota->size(), conf->get_number_of_layers());
}

TEST_F(Rota_Fixture, test_SpaceMain) {
    std::vector<rota::RotaLayer *> *rota = &this->rotaSource;

    RotaLayer *temp = nullptr;

    for (RotaLayer *layer : *rota) {
        if (temp != nullptr && layer->getMode()->isMainMode &&
            temp->getMode()->isMainMode) {
            ASSERT_TRUE(layer->getMode() != temp->getMode());
        }
        temp = layer;
    }
}

TEST_F(Rota_Fixture, test_max_time_same_team) {
    std::vector<rota::RotaLayer *> *rota = &this->rotaSource;
    std::vector<RotaTeam *> teamBlue;
    std::vector<RotaTeam *> teamRed;

    int temp = 0;
    for (RotaLayer *layer : *rota) {
        if (temp) {
            teamBlue.push_back(layer->getTeam(1));
            teamRed.push_back(layer->getTeam(0));
            temp = 0;
        } else {
            teamBlue.push_back(layer->getTeam(0));
            teamRed.push_back(layer->getTeam(1));
            temp = 1;
        }
    }

    RotaTeam *curr = nullptr;
    int counter;
    int i = 0;
    for (RotaTeam *team : teamBlue) {
        if (curr != team) {
            curr = team;
            counter = 0;
        }
        counter++;
        if (counter > conf->get_max_same_team()) {
            std::cout << i << std::endl;
        }
        ASSERT_LE(counter, conf->get_max_same_team());
        i++;
    }

    for (RotaTeam *team : teamRed) {
        if (curr != team) {
            curr = team;
            counter = 0;
        }
        counter++;
        ASSERT_LE(counter, conf->get_max_same_team());
    }
}

TEST_F(Rota_Fixture, test_layer_lockTime) {
    std::vector<rota::RotaLayer *> *rota = &this->rotaSource;
    std::map<RotaLayer *, int> counter;
    std::map<RotaLayer *, int>::iterator it;

    int i = 0;
    for (RotaLayer *layer : *rota) {
        it = counter.find(layer);
        if (it != counter.end()) {
            ASSERT_GE(i - counter[layer], conf->get_layer_locktime());
        }
        counter[layer] = i;
        i++;
    }
}

TEST_F(Rota_Fixture, test_map_lockTime) {
    std::vector<rota::RotaLayer *> *rota = &this->rotaSource;
    std::map<RotaMap *, int> counter;
    std::map<RotaMap *, int>::iterator it;

    int i = 0;
    for (RotaLayer *layer : *rota) {
        it = counter.find(layer->getMap());
        if (it != counter.end()) {
            ASSERT_GE(i - counter[layer->getMap()], conf->get_biom_spacing());
        }
        counter[layer->getMap()] = i;
        i++;
    }
}

TEST_F(Rota_Fixture, test_seed_map_count) {
    std::vector<rota::RotaLayer *> *rota = &this->rotaSource;

    int counter = 0;
    for (RotaLayer *layer : *rota) {
        if (layer->getMode()->name.compare("Seed") == 0) {
            counter++;
        }
    }
    ASSERT_EQ(counter, conf->get_seed_layer());
}

TEST_F(Rota_Fixture, test_main_mode_after_seed) {
    std::vector<rota::RotaLayer *> *rota = &this->rotaSource;

    RotaLayer *temp = nullptr;
    for (RotaLayer *layer : *rota) {
        if (layer->getMode()->name.compare("Seed")) {
            temp = layer;
        } else {
            if (temp != nullptr) {
                ASSERT_TRUE(layer->getMode()->isMainMode);
                temp = nullptr;
            }
        }
    }
}
