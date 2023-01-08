#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/json.hpp>
#include <httplib.h>

#include "utils.hpp"

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

int getLayers(std::string url, std::string req,
              std::map<std::string, RotaLayer *> *layers) {

    namespace json = boost::json;

    httplib::Client cli(url);
    auto res = cli.Get(req);
    if (res->status != 200) {
        throw std::runtime_error("Error while getting layer " + url + req +
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

void injectLayerInfo(std::string url, std::string req,
                     std::map<std::string, rota::RotaLayer *> *layers,
                     std::map<std::string, RotaMode *> *modes,
                     std::map<std::string, RotaTeam *> *teams) {

    namespace json = boost::json;

    httplib::Client cli(url);
    auto res = cli.Get(req);
    if (res->status != 200) {
        throw std::runtime_error(
            "Error while getting additional layer information " + url + req +
            " status:" + std::to_string(res->status));
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
                RotaMode *mode = new RotaMode(modeName);
                (*modes)[modeName] = mode;
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

} // namespace rota
