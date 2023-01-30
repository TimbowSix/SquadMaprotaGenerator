/**
 * @file Config
 * @brief Object representation of RotaGenerator config
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */
#pragma once

#include <boost/json.hpp>
#include <map>
#include <string>
#include <vector>

#include "RotaModePool.hpp"

namespace rota {
class RotaConfig {
  private:
    /**
     * @brief number of layers per rotation (incl. Seed layers)
     */
    unsigned int number_of_layers;
    /**
     * @brief number of seed layers at the top of the rotation
     */
    unsigned int seed_layer;
    /**
     * @brief url to the API containing all layers and their votes
     */
    std::string layer_vote_api_url;
    /**
     * @brief url to the API containing all layers and their teams
     */
    std::string team_api_url;
    /**
     * @brief list of maps to be used in the rotation
     */
    std::vector<std::string> maps;
    /**
     * @brief locktime; how long a map and its neighbors are beeing locked
     */
    unsigned int biom_spacing;
    /**
     * @brief how long a layer should be locked after being drawn
     */
    unsigned int layer_locktime;
    /**
     * @brief how many times the same team can come in a row
     */
    unsigned int max_same_team;
    /**
     * @brief minimum distance between two layers
     *        layers within this distance are being locked for {biom_spacing}
     * turns
     */
    float min_biom_distance;
    /**
     * @brief sigmoind slope of mapvote
     */
    float mapvote_slope;
    /**
     * @brief sigmoind shift of mapvote
     */
    float mapvote_shift;
    /**
     * @brief sigmoind slope of layervote
     */
    float layervote_slope;
    /**
     * @brief sigmoind shift of layervote
     */
    float layervote_shift;

    /**
     * @brief minimum number of turns between any NOT main pool modes
     */
    unsigned int pool_spacing;
    /**
     * @brief whether the same main mode is allowed to come directly after
     * another
     */
    bool space_main;
    /**
     * @brief map of all mode pools
     */
    std::map<std::string, RotaModePool *> pools;
    /**
     * @brief map of all modes
     */
    std::map<std::string, RotaMode *> modes;

  public:
    /**
     * @brief directly parse config json object from given path
     *
     * @param object config json path
     */
    RotaConfig(std::string path);

    // getter / setter

    unsigned int get_number_of_layers();
    void set_number_of_layers(unsigned int value);

    unsigned int get_seed_layer();
    void set_seed_layer(unsigned int value);

    std::string get_layer_vote_api_url();
    void set_layer_vote_api_url(std::string value);

    std::string get_team_api_url();
    void set_team_api_url(std::string value);

    std::vector<std::string> *get_maps();
    void set_maps(std::vector<std::string> value);

    unsigned int get_biom_spacing();
    void set_biom_spacing(unsigned int value);

    unsigned int get_layer_locktime();
    void set_layer_locktime(unsigned int value);

    unsigned int get_max_same_team();
    void set_max_same_team(unsigned int value);

    float get_min_biom_distance();
    void set_min_biom_distance(float value);

    float get_mapvote_slope();
    void set_mapvote_slope(float value);

    unsigned int get_mapvote_shift();
    void set_mapvote_shift(unsigned int value);

    float get_layervote_slope();
    void set_layervote_slope(float value);

    unsigned int get_layervote_shift();
    void set_layervote_shift(unsigned int value);

    unsigned int get_pool_spacing();
    void set_pool_spacing(unsigned int value);

    bool get_space_main();
    void set_space_main(bool value);

    std::map<std::string, RotaModePool *> *get_pools();
    void set_pools(std::map<std::string, RotaModePool *> value);

    std::map<std::string, RotaMode *> *get_modes();
    void set_modes(std::map<std::string, RotaMode *> value);
};
} // namespace rota
