#include "RotaConfig.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "RotaModePool.hpp"
#include "dataParsing.hpp"

namespace rota {
RotaConfig::RotaConfig(std::string path) {
    const std::filesystem::path configFile{path};
    std::ifstream ifs(configFile);
    std::string data(std::istreambuf_iterator<char>{ifs}, {});
    ifs.close();
    boost::json::object pData = boost::json::parse(data).get_object();

    parseModes(&pData, &this->pools, &this->modes);
    number_of_layers = pData["number_of_layers"].as_int64();
    seed_layer = pData["seed_layer"].as_int64();
    layer_vote_api_url = pData["layer_vote_api_url"].as_string();
    team_api_url = pData["team_api_url"].as_string();
    biom_spacing = pData["biom_spacing"].as_int64();
    layer_locktime = pData["layer_locktime"].as_int64();
    max_same_team = pData["max_same_team"].as_int64();
    min_biom_distance = pData["min_biom_distance"].is_double()
                            ? pData["min_biom_distance"].as_double()
                            : pData["min_biom_distance"].as_int64();
    mapvote_slope = pData["mapvote_slope"].is_double()
                        ? pData["mapvote_slope"].as_double()
                        : pData["mapvote_slope"].as_int64();
    mapvote_shift = pData["mapvote_shift"].is_double()
                        ? pData["mapvote_shift"].as_double()
                        : pData["mapvote_shift"].as_int64();
    layervote_slope = pData["layervote_slope"].is_double()
                          ? pData["layervote_slope"].as_double()
                          : pData["layervote_slope"].as_int64();
    layervote_shift = pData["layervote_shift"].is_double()
                          ? pData["layervote_shift"].as_double()
                          : pData["layervote_shift"].as_int64();
    pool_spacing = pData["mode_distribution"].at("pool_spacing").as_int64();
    space_main = pData["mode_distribution"].at("space_main").as_bool();
    // TODO parse maps list
    boost::json::array usedMapsRaw = pData["maps"].as_array();
    for (int i = 0; i < usedMapsRaw.size(); i++) {
        std::string map = (std::string)usedMapsRaw[i].as_string();
        maps.push_back(map);
    }
}
// getter / setter
unsigned int RotaConfig::get_number_of_layers() { return number_of_layers; }
void RotaConfig::set_number_of_layers(unsigned int value) {
    this->number_of_layers = value;
}

unsigned int RotaConfig::get_seed_layer() { return seed_layer; }
void RotaConfig::set_seed_layer(unsigned int value) {
    this->seed_layer = value;
}

std::string RotaConfig::get_layer_vote_api_url() { return layer_vote_api_url; }
void RotaConfig::set_layer_vote_api_url(std::string value) {
    this->layer_vote_api_url = value;
}

std::string RotaConfig::get_team_api_url() { return team_api_url; }
void RotaConfig::set_team_api_url(std::string value) {
    this->team_api_url = value;
}

std::vector<std::string> *RotaConfig::get_maps() { return &maps; }
void RotaConfig::set_maps(std::vector<std::string> value) {
    this->maps = value;
}

unsigned int RotaConfig::get_biom_spacing() { return biom_spacing; }
void RotaConfig::set_biom_spacing(unsigned int value) {
    this->biom_spacing = value;
}

unsigned int RotaConfig::get_layer_locktime() { return layer_locktime; }
void RotaConfig::set_layer_locktime(unsigned int value) {
    this->layer_locktime = value;
}

unsigned int RotaConfig::get_max_same_team() { return max_same_team; }
void RotaConfig::set_max_same_team(unsigned int value) {
    this->max_same_team = value;
}

float RotaConfig::get_min_biom_distance() { return min_biom_distance; }
void RotaConfig::set_min_biom_distance(float value) {
    this->min_biom_distance = value;
}

float RotaConfig::get_mapvote_slope() { return mapvote_slope; }
void RotaConfig::set_mapvote_slope(float value) { this->mapvote_slope = value; }

unsigned int RotaConfig::get_mapvote_shift() { return mapvote_shift; }
void RotaConfig::set_mapvote_shift(unsigned int value) {
    this->mapvote_shift = value;
}

float RotaConfig::get_layervote_slope() { return layervote_slope; }
void RotaConfig::set_layervote_slope(float value) {
    this->layervote_slope = value;
}

unsigned int RotaConfig::get_layervote_shift() { return layervote_shift; }
void RotaConfig::set_layervote_shift(unsigned int value) {
    this->layervote_shift = value;
}

unsigned int RotaConfig::get_pool_spacing() { return pool_spacing; }
void RotaConfig::set_pool_spacing(unsigned int value) {
    this->pool_spacing = value;
}

bool RotaConfig::get_space_main() { return space_main; }
void RotaConfig::set_space_main(bool value) { this->space_main = value; }

std::map<std::string, RotaModePool *> *RotaConfig::get_pools() {
    return &pools;
}
void RotaConfig::set_pools(std::map<std::string, RotaModePool *> value) {
    this->pools = value;
}

std::map<std::string, RotaMode *> *RotaConfig::get_modes() { return &modes; }
void RotaConfig::set_modes(std::map<std::string, RotaMode *> value) {
    this->modes = value;
}

} // namespace rota
