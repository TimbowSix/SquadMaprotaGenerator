#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/json.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <httplib.h>

#include <iostream>
#include <regex>
#include <tuple>

#include "GlobalConfig.hpp"
#include "RotaMap.hpp"
#include "utils.hpp"

namespace rota {

int weightedChoice(std::vector<float> *weights, rotaRNG &rng) {
    float weightSum = 0.0;
    weightSum = std::accumulate(weights->begin(), weights->end(), 0.0);
    weightSum =
        roundf(weightSum * 100) /
        100; // round to 2 decimal places to account floating point error
    if (weightSum != 1) {
        throw std::invalid_argument("Weights do not sum to 1");
    }
    std::uniform_real_distribution<> dis(0, 1.0);
    float randomValue = dis(rng);
    float currentValue = 0;
    for (int i = 0; i < weights->size(); i++) {
        currentValue += weights->at(i);
        if (randomValue <= currentValue) {
            return i;
        }
    }
    // return last value if currentValue cant reach 1 because of rounding errors
    return weights->size() - 1;
}

int choice(int length, rotaRNG &rng) {
    assert(length >= 1);
    std::uniform_int_distribution<uint32_t> dis(0, length - 1);
    return dis(rng);
}

void normalize(std::vector<float> *arr, float *sum) {
    float arrSum;
    if (sum != NULL) {
        arrSum = *sum;
    } else {
        arrSum = std::accumulate(arr->begin(), arr->end(), 0.0);
    }

    if (arrSum == 0) {
        for (int i = 0; i < arr->size(); i++) {
            (*arr)[i] = 0;
        }
    } else {
        for (int i = 0; i < arr->size(); i++) {
            (*arr)[i] = (*arr)[i] / arrSum;
        }
    }
}

void normalize(std::vector<float> *arr) { normalize(arr, NULL); }

float sigmoid(float x, float slope, float shift) {
    float arg = slope * (x + shift);
    return 1 / (1 + exp(-arg));
}

int getLayers(std::string url, std::map<std::string, RotaLayer *> *layers) {

    namespace json = boost::json;

    std::string baseUrl;
    std::string subUrl;
    std::tie(baseUrl, subUrl) = parseUrl(url);

    httplib::Client cli(baseUrl);
    auto res = cli.Get(subUrl);
    if (res->status != 200) {
        throw std::runtime_error("Error while getting layer " + baseUrl +
                                 subUrl +
                                 " status:" + std::to_string(res->status));
        return 0;
    }
    json::array layerData = json::parse(res->body).get_array();
    int counter = 0;
    for (int i = 0; i < layerData.size(); i++) {
        json::object obj = layerData[i].get_object();
        if (!obj["isExcluded"].as_bool()) {
            int upVotes = obj["upvotes"].as_int64();
            int downVotes = obj["downvotes"].as_int64();
            RotaLayer *layer =
                new RotaLayer((std::string)obj["layer"].as_string(),
                              (float)(upVotes - downVotes));
            (*layers)[layer->getName()] = layer;
            counter++;
        }
    }

    return 0;
}

void injectLayerInfo(std::string url,
                     std::map<std::string, rota::RotaLayer *> *layers,
                     std::map<std::string, RotaMode *> *modes,
                     std::map<std::string, RotaTeam *> *teams) {

    namespace json = boost::json;

    std::string baseUrl;
    std::string subUrl;
    std::tie(baseUrl, subUrl) = parseUrl(url);

    httplib::Client cli(baseUrl);
    auto res = cli.Get(subUrl);
    if (res->status != 200) {
        throw std::runtime_error(
            "Error while getting additional layer information " + baseUrl +
            subUrl + " status:" + std::to_string(res->status));
        return;
    }

    json::array data = json::parse(res->body).get_array();

    for (int i = 0; i < data.size(); i++) {
        json::object obj = data[i].get_object();

        std::string layerName = (std::string)obj["id"].as_string();
        std::string modeName = (std::string)obj["gamemode"].as_string();
        std::string teamOneName = (std::string)obj["teamOne"].as_string();
        std::string teamTwoName = (std::string)obj["teamTwo"].as_string();

        if (layers->find(layerName) != layers->end()) {

            if (modes->find(modeName) == modes->end()) {
                // create non existing mode
                continue; // skip unused mode
                /*
                RotaMode *mode = new RotaMode(modeName);
                (*modes)[modeName] = mode;
                */
            }
            (*layers)[layerName]->setMode((*modes)[modeName]);

            if (teams->find(teamOneName) == teams->end()) {
                // create non existing team
                RotaTeam *team = new RotaTeam(teamOneName);
                (*teams)[teamOneName] = team;
            }
            (*layers)[layerName]->setTeam((*teams)[teamOneName], 0);

            if (teams->find(teamTwoName) == teams->end()) {
                // create non existing team
                RotaTeam *team = new RotaTeam(teamTwoName);
                (*teams)[teamTwoName] = team;
            }
            (*layers)[layerName]->setTeam((*teams)[teamTwoName], 1);
        }
    }
}

std::tuple<std::string, std::string> parseUrl(std::string url) {
    std::regex pattern("^(https?://[a-zA-z]+.[a-zA-z]+.[a-zA-z]+)(.*)$");
    std::smatch match;
    std::regex_match(url, match, pattern);

    std::string baseUrl = match[1];
    std::string subUrl = match[2];

    return std::make_tuple(baseUrl, subUrl);
}

void setNeighbour(std::vector<RotaMap *> *maps, float neighbourDist) {
    for (int i = 0; i < maps->size(); i++) {
        for (int j = 0; j < maps->size(); j++) {
            if (getMapDist((*maps)[i], (*maps)[j]) <= neighbourDist) {
                (*maps)[i]->addNeighbour((*maps)[j]);
            }
        }
    }
}

float getMapDist(RotaMap *map0, RotaMap *map1) {

    namespace ublas = boost::numeric::ublas;
    float a =
        ublas::inner_prod(*map0->getBiomValues(), *map1->getBiomValues()) /
        (ublas::norm_2(*map0->getBiomValues()) *
         ublas::norm_2(*map1->getBiomValues()));

    if (a <= -1.0) {
        return boost::math::float_constants::pi;
    } else if (a >= 1.0) {
        return 0;
    } else {
        return acos(a);
    }
}

void printMapNeighbor(std::vector<RotaMap *> *maps) {
    for (RotaMap *map : *maps) {
        std::cout << "-- " << map->getName() << " --" << std::endl;
        std::cout << "----------------------" << std::endl;
        for (RotaMap *nMap : *map->getNeighbor()) {
            std::cout << nMap->getName() << std::endl;
        }
        std::cout << std::endl;
    }
}

void printMemColonel(std::vector<RotaMap *> *maps) {
    std::cout << "Map \r\t\t\t locked time" << std::endl;

    for (RotaMap *map : *maps) {
        std::cout << map->getName() << "\r\t\t\t " << map->getCurrLockTime()
                  << std::endl;
    }
}

} // namespace rota
