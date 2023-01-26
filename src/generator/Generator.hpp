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
     * @brief counter for each mode, how many maps are available
     *        the first number is the count of maps which are available through
     *        locks the second number is the count of maps with have available
     *        layer
     */
    std::map<RotaMode *, int> availableLayerMaps;
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
    std::vector<RotaMode *> ModesHistory;
    /**
     * @brief all drawn layers
     */
    std::vector<RotaLayer *> rotation;
    /**
     * @brief all drawn maps in order
     */
    std::vector<RotaMap *> MapsHistory;
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
    std::vector<RotaMode *> modeBuffer;

    /**
     * @brief list of normally choosen mode pools
     */
    std::vector<RotaModePool *> defaultModePools;

    /**
     * @brief precalculated weights of default mode pools
     */
    std::vector<float> defaultPoolWeights;

    /**
     * @brief counter for determine the distance to the last non main mode
     */
    int lastNonMainMode;

    /**
     * @brief pre allocated map of a list of all mode weights
     */
    std::map<RotaModePool *, std::vector<float>> modeWeights;

    /**
     * @brief pre allocated map of a modePool to a mode list
     */
    std::map<RotaModePool *, std::vector<RotaMode *>> poolToModeList;

    /**
     * @brief index of the next main mode if space_main is active
     */
    int nextMainModeIndex;

    /**
     * @brief seed of rota
     */
    time_t seed;

  public:
    Generator(RotaConfig *config);

    /**
     * @brief Selects a random game mode based on the modes in the mode pools
     * and the corresponding probabilities set in the configuration
     * considering the lock maps and layer
     *
     * @param customPool set pool to draw mode from
     * @param ignoreModeBuff ignores the mode buffer while choosing a mode
     * @param depth this function calls itself if necessary to prevent a endless
     * loop the function throw a error if a depth is reached
     * @returns pointer to choosen mode
     */
    RotaMode *chooseMode(RotaModePool *customPool, bool ignoreModeBuff,
                         int depth);

    /**
     * @brief chooses a random map from maps with probabilities given by their
     * weights // TODO hier weiter machen ...
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
    void reset(std::vector<RotaLayer *> *pastLayers, time_t seed);
    /**
     * @brief resets all temporary values used for generation
     *        parses and sets given previous layers
     *
     * @throws std::out_of_range if a layer name does not exists]
     */
    void reset(std::vector<std::string> *pastLayers);

    bool mapsAvailable(RotaMode *mode);
    time_t getSeed();
};
} // namespace rota
