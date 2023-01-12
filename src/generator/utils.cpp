#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/json.hpp>
#include <httplib.h>

#include "utils.hpp"
#include <regex>
#include <tuple>

#include <iostream>

#include "RotaMap.hpp"

namespace rota {

int weightedChoice(std::vector<float> *weights) {
    float weightSum = std::accumulate(weights->begin(), weights->end(), 0.0);
    weightSum =
        roundf(weightSum * 100) /
        100; // round to 2 decimal places to account floating point error
    if (weightSum != 1) {
        throw std::invalid_argument("Weights do not sum to 1");
    }
    float randomValue = (float)rand() / RAND_MAX;
    float currentValue = 0;
    for (int i = 0; i < weights->size(); i++) {
        currentValue += weights->at(i);
        if (randomValue <= currentValue) {
            return i;
        }
    }
    return -1;
}

int choice(int length) {
    assert(length >= 1);
    return rand() % length;
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

float sigmoid(float x, float slope, float shift) {
    float arg = slope * (x + shift);
    return 1 / (1 + exp(-arg));
}

int getLayers(std::string url, std::vector<RotaLayer *> *layers) {

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
            layers->push_back(layer);
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

void setNeibour(std::vector<RotaMap *> *maps, float neighbourDist) {}

template <typename T> struct square {
    T operator()(const T &Left, const T &Right) const {
        return (Left + Right * Right);
    }
};

float getMapDist(RotaMap *map0, RotaMap *map1) {

    float absVal0 = std::sqrt(std::accumulate(map0->getBiomValues()->begin(),
                                              map0->getBiomValues()->end(), 0,
                                              square<float>()));

    for (int i = 0; i < map0->getBiomValues()->size(); i++) {
    }
}

} // namespace rota
