#pragma once

#include "RotaConfig.hpp"
#include "RotaLayer.hpp"
#include "RotaMap.hpp"
#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include "RotaTeam.hpp"
#include <boost/json.hpp>
#include <map>
#include <vector>

namespace rota {
class Generator {
  private:
    RotaConfig *config;
    /**
     * @brief maps all available maps to their name
     */
    std::map<std::string, RotaMap *> mapsByName;
    /**
     * @brief list of all maps
     */
    std::vector<RotaMap *> maps;

    /**
     * @brief a map from mode to a list of maps
     */
    std::map<std::string, std::vector<RotaMap *>> modeToMapList;
    /**
     * @brief history for team 1 or team 2
     */
    std::vector<RotaTeam *> teamHistory[2];
    /**
     * @brief a counter for both team with increases if a team is
     * playing the same team again
     */
    int sameTeamCounter[2];
    /**
     * @brief maps all available ModePools to their name
     */
    std::map<std::string, RotaModePool *> modePools;
    /**
     * @brief maps all modes to their name
     */
    std::map<std::string, RotaMode *> modes;
    /**
     * @brief maps all layers to their name
     */
    std::map<std::string, RotaLayer *> layers;
    /**
     * @brief maps all teams to their name
     */
    std::map<std::string, RotaTeam *> teams;
    /**
     * @brief all drawn modes in order
     */
    std::vector<RotaMode *> latestModes;
    /**
     * @brief all drawn layers
     */
    std::vector<RotaLayer *> rotation;
    /**
     * @brief all drawn maps in order
     */
    std::vector<RotaMap *> latestMaps;
    /**
     * @brief maps bluefor teams on containing layers
     */
    std::map<RotaTeam *, std::vector<RotaLayer *>> blueforTeams;
    /**
     * @brief maps opfor teams on containing layers
     */
    std::map<RotaTeam *, std::vector<RotaLayer *>> opforTeams;
    /**
     * @brief buffer currently unavailable modes
     */
    RotaMode *modeBuffer;

  public:
    Generator(RotaConfig *config);

    /**
     * @brief Selects a random game mode based on the modes in the mode pools
     * and the corresponding probabilities set in the configuration
     *
     * @param useLatestModes usage of latest modes for mode locking
     * @param customPool set pool to draw mode from
     * @returns pointer to choosen mode
     */
    RotaMode *chooseMode(bool useLatestModes, RotaModePool *customPool);

    /**
     * @brief chooses a random map from maps with probabilities given by their
     * weight for a given mode uses fallback mode of main pool if no maps are
     * available for given mode reduces map locktimes if no maps for either mode
     * or fallback mode are available
     *
     * @param mode mode to draw from
     * @returns chosen map
     */
    RotaMap *chooseMap(RotaMode *mode);

    /**
     * @brief chooses a random layer from a given map for a given mode
     *
     * @param map map to choose from
     * @param mode mode to choose from
     * @return chosen layer
     */
    RotaLayer *chooseLayerFromMap(RotaMap *map, RotaMode *mode);

    void lockTeams();

    void decreaseMapLocktimes();
    void decreaseLayerLocktimes();

    /**
     * @brief Creates a new rotation based on the parameters set in the
     * configuration.
     *
     * @returns rotation of layers
     */
    // std::vector<RotaLayer*> generateRota();
    void generateRota();

    /**
     * @brief resets all temporary values used for generation
     */
    void reset();
    /**
     * @brief resets all temporary values used for generation and sets given
     * previous layers
     */
    void reset(std::vector<RotaLayer *> *pastLayers);
    /**
     * @brief resets all temporary values used for generation
     *        parses and sets given previous layers
     */
    void reset(std::vector<std::string> *pastLayers);
};
} // namespace rota
