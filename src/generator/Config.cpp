#include "Config.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "RotaModePool.hpp"
#include "dataParsing.hpp"

namespace rota
{
Config::Config(std::string path){
    const std::filesystem::path configFile{path};
    std::ifstream ifs(configFile);
    std::string data(std::istreambuf_iterator<char>{ifs}, {});
    boost::json::object pData = boost::json::parse(data).get_object();

    parseModes(&pData, &this->pools, &this->modes);
    //TODO Typecasting JSON Integer/Float
    number_of_rotas = pData["number_of_rotas"].as_int64();
    number_of_layers = pData["number_of_layers"].as_int64();
    seed_layer = pData["seed_layer"].as_int64();
    update_layers = pData["update_layers"].as_bool();
    update_teams = pData["update_teams"].as_bool();
    output_path = pData["output_path"].as_string();
    layer_vote_api_url = pData["layer_vote_api_url"].as_string();
    team_api_url = pData["team_api_url"].as_string();
    biom_spacing = pData["biom_spacing"].as_int64();
    layer_locktime = pData["layer_locktime"].as_int64();
    max_same_team = pData["max_same_team"].as_int64();
    min_biom_distance = pData["min_biom_distance"].as_double();
    mapvote_slope = pData["mapvote_slope"].as_double();
    mapvote_shift = pData["mapvote_shift"].as_double();
    layervote_slope = pData["layervote_slope"].as_double();
    layervote_shift = pData["layervote_shift"].as_double();
    save_expected_map_dist = pData["save_expected_map_dist"].as_bool();
    use_lock_time_modifier = pData["use_lock_time_modifier"].as_bool();
    auto_optimize = pData["auto_optimize"].as_bool();
    fix_unavailables = pData["fix_unavailables"].as_bool();
    pool_spacing = pData["pool_spacing"].as_int64();
    space_main = pData["space_main"].as_bool();
}
// getter / setter
unsigned int Config::get_number_of_rotas() { return number_of_rotas; }
void Config::set_number_of_rotas(unsigned int value) { this->number_of_rotas = value; }

unsigned int Config::get_number_of_layers() { return number_of_layers; }
void Config::set_number_of_layers(unsigned int value) { this->number_of_layers = value; }

unsigned int Config::get_seed_layer()  { return seed_layer; }
void Config::set_seed_layer(unsigned int value) { this->seed_layer = value; }

bool Config::get_update_layers() { return update_layers; }
void Config::set_update_layers(bool value) { this->update_layers = value; }

bool Config::get_update_teams() { return update_teams; }
void Config::set_update_teams(bool value) { this->update_teams = value; }

std::string Config::get_output_path() { return output_path; }
void Config::set_output_path(std::string value) { this->output_path = value; }

std::string Config::get_layer_vote_api_url() { return layer_vote_api_url; }
void Config::set_layer_vote_api_url(std::string value) { this->layer_vote_api_url = value; }

std::string Config::get_team_api_url() { return team_api_url; }
void Config::set_team_api_url(std::string value) { this->team_api_url = value; }

const std::vector<std::string>* Config::get_maps() const { return &maps; }
void Config::set_maps(std::vector<std::string> value) { this->maps = value; }

unsigned int Config::get_biom_spacing() { return biom_spacing; }
void Config::set_biom_spacing(unsigned int value) { this->biom_spacing = value; }

unsigned int Config::get_layer_locktime() { return layer_locktime; }
void Config::set_layer_locktime(unsigned int value) { this->layer_locktime = value; }

unsigned int Config::get_max_same_team() { return max_same_team; }
void Config::set_max_same_team(unsigned int value) { this->max_same_team = value; }

double Config::get_min_biom_distance() { return min_biom_distance; }
void Config::set_min_biom_distance(double value) { this->min_biom_distance = value; }

double Config::get_mapvote_slope() { return mapvote_slope; }
void Config::set_mapvote_slope(double value) { this->mapvote_slope = value; }

unsigned int Config::get_mapvote_shift() { return mapvote_shift; }
void Config::set_mapvote_shift(unsigned int value) { this->mapvote_shift = value; }

double Config::get_layervote_slope() { return layervote_slope; }
void Config::set_layervote_slope(double value) { this->layervote_slope = value; }

unsigned int Config::get_layervote_shift() { return layervote_shift; }
void Config::set_layervote_shift(unsigned int value) { this->layervote_shift = value; }

bool Config::get_save_expected_map_dist() { return save_expected_map_dist; }
void Config::set_save_expected_map_dist(bool value) { this->save_expected_map_dist = value; }

bool Config::get_use_lock_time_modifier() { return use_lock_time_modifier; }
void Config::set_use_lock_time_modifier(bool value) { this->use_lock_time_modifier = value; }

bool Config::get_auto_optimize() { return auto_optimize; }
void Config::set_auto_optimize(bool value) { this->auto_optimize = value; }

bool Config::get_fix_unavailables() { return fix_unavailables; }
void Config::set_fix_unavailables(bool value) { this->fix_unavailables = value; }

const std::map<std::string, RotaModePool *>* Config::get_pools() const { return &pools; }
void Config::set_pools(std::map<std::string, RotaModePool *> value){ this->pools = value; }

} // namespace rota
