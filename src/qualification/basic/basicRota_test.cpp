#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <Generator.hpp>
#include <GlobalConfig.hpp>
#include <RotaConfig.hpp>
#include <RotaLayer.hpp>

using namespace rota;

TEST(Rota_Test, test_SpaceMain) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();

    RotaLayer *temp = nullptr;

    for (RotaLayer *layer : *rota) {
        if (temp != nullptr && layer->getMode()->isMainMode &&
            temp->getMode()->isMainMode) {
            ASSERT_TRUE(layer->getMode() != temp->getMode());
        }
        temp = layer;
    }
}

TEST(Rota_Test, test_numberRotaLayer) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();
    ASSERT_EQ(rota->size(), conf.get_number_of_layers());
}

TEST(Rota_Test, test_max_time_same_team) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();
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
        if (counter > conf.get_max_same_team()) {
            std::cout << i << std::endl;
        }
        ASSERT_LE(counter, conf.get_max_same_team());
        i++;
    }

    for (RotaTeam *team : teamRed) {
        if (curr != team) {
            curr = team;
            counter = 0;
        }
        counter++;
        ASSERT_LE(counter, conf.get_max_same_team());
    }
}

TEST(Rota_Test, test_layer_lockTime) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();
    std::map<RotaLayer *, int> counter;
    std::map<RotaLayer *, int>::iterator it;

    int i = 0;
    for (RotaLayer *layer : *rota) {
        it = counter.find(layer);
        if (it != counter.end()) {
            ASSERT_GE(i - counter[layer], conf.get_layer_locktime());
        }
        counter[layer] = i;
        i++;
    }
}

TEST(Rota_Test, test_map_lockTime) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();
    std::map<RotaMap *, int> counter;
    std::map<RotaMap *, int>::iterator it;

    int i = 0;
    for (RotaLayer *layer : *rota) {
        it = counter.find(layer->getMap());
        if (it != counter.end()) {
            ASSERT_GE(i - counter[layer->getMap()], conf.get_biom_spacing());
        }
        counter[layer->getMap()] = i;
        i++;
    }
}

TEST(Rota_Test, test_seed_map_count) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();

    int counter = 0;
    for (RotaLayer *layer : *rota) {
        if (layer->getMode()->name.compare("Seed") == 0) {
            counter++;
        }
    }
    ASSERT_EQ(counter, conf.get_seed_layer());
}

TEST(Rota_Test, test_main_mode_after_seed) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();

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

TEST(Rota_Test, test_distribution) {
    RotaConfig conf(std::string(CONFIG_PATH) + "config.json");

    conf.set_number_of_layers(100000);
    conf.set_seed_layer(1);
    conf.set_space_main(true);

    Generator gen(&conf);
    gen.generateRota();

    std::vector<rota::RotaLayer *> *rota = gen.getRota();
    std::map<RotaModePool *, int> counter;
    std::map<RotaModePool *, float> res;
    std::map<RotaMode *, int> counterMode;
    std::map<RotaMode *, float> resMode;

    for (RotaLayer *layer : *rota) {
        if (layer->getMode()->modePool != nullptr) {
            counter[layer->getMode()->modePool] = 0;
        }
        counterMode[layer->getMode()] = 0;
    }

    for (RotaLayer *layer : *rota) {
        if (layer->getMode()->modePool != nullptr) {
            counter[layer->getMode()->modePool]++;
        }
        counterMode[layer->getMode()]++;
    }

    for (auto const &a : counter) {
        res[a.first] = (float)a.second / rota->size();
    }

    for (auto const &a : counterMode) {
        resMode[a.first] = (float)a.second / rota->size();
    }

    for (auto const &x : res) {
        ASSERT_NEAR((*conf.get_pools()).at(x.first->name)->probability,
                    x.second, 0.01)
            << "pool distribution off. Pool:" << x.first->name << " expected "
            << (*conf.get_pools()).at(x.first->name)->probability << " actual "
            << x.second;
    }

    for (auto const &x : resMode) {
        ASSERT_NEAR((*conf.get_modes()).at(x.first->name)->probability,
                    x.second, 0.01)
            << "mode distribution off. Mode" << x.first->name << " expected "
            << (*conf.get_modes()).at(x.first->name)->probability << " actual "
            << x.second;
    }
}