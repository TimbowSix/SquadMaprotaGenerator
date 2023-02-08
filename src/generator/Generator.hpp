#pragma once

#include <boost/json.hpp>
#include <map>
#include <random>
#include <sys/types.h>
#include <vector>

#include "OptimizerData.hpp"
#include "RotaConfig.hpp"
#include "RotaLayer.hpp"
#include "RotaMap.hpp"
#include "RotaMode.hpp"
#include "RotaModePool.hpp"
#include "RotaTeam.hpp"

namespace rota {

typedef struct MemoryColonelState MemoryColonelState;

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
     * @brief emulates the side switch of teams
     */
    int currTeamIndex[2] = {0, 1};
    /**
     * @brief helper var to mem last Team
     */
    RotaTeam *lastTeam[2];

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
    std::map<std::string, RotaModePool *> *modePools;
    /**
     * @brief maps all modes to their name
     */
    std::map<std::string, RotaMode *> *modes;
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
    u_int32_t seed;
    /**
     * @brief random number generator
     */
    rotaRNG rng;

    /**
     * @brief hash over all layer names
     */
    size_t layerHash;

    /**
     * @brief generates a seed and sets the object variable
     */
    void generateSeed();

  public:
    Generator(RotaConfig *config);
    ~Generator();

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
     *        weights
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
     * @param withSeed generates a rota with/without seed
     * @param length   length of rota
     * @returns rotation of layers
     */
    void generateRota();
    void generateRota(bool withSeed, int length);

    /**
     * @brief generates x unique Layer based off the current generator state
     *        this function generate suggestions for a next layer
     *
     * @param out list with get filled with layer suggestions
     * @param count number of suggestion to generate
     */
    void generateOffer(std::vector<RotaLayer *> *out, int count);

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
     *
     * @throws std::out_of_range if a layer name does not exists]
     */
    void reset(std::vector<std::string> *pastLayers);

    /**
     * @brief show if maps are for a specific mode are available
     */
    bool mapsAvailable(RotaMode *mode);

    /**
     * @brief set map weights for all maps and modes
     *
     * @param data data from optimizer
     * @param mode for with mode are the new MapWeights
     */
    void setMapWeights(OptDataOut *data, RotaMode *mode);

    /**
     * @brief creates Data for Optimizer
     *
     * @param data OptData to fill
     * @param mode
     */
    void packOptData(OptDataIn *data, RotaMode *mode);

    /**
     * @brief creates a hash over all layer names
     *        stores it locally
     */
    void generateLayerHash();

    // getter & setter
    u_int32_t getSeed();
    std::vector<RotaLayer *> *getRota();
    void getState(MemoryColonelState *state);
    /**
     * @brief only for testing
     */
    void setRandomMapWeights(RotaMode *mode);
    std::map<std::string, RotaMode *> *getModes();
    std::map<std::string, RotaLayer *> *getLayerMap();
    size_t getLayerHash();
};

struct MemoryColonelState {
    /**
     * @brief all locktimes
     */
    std::vector<int> mapState;
    /**
     * @brief all locktimes
     */
    std::vector<int> layerState;
    /**
     * @brief lastNonMainMode, sameTeamCounter, currTeamIndex
     */
    std::vector<int> genState;
    /**
     * @brief last choosen teams of rota
     */
    RotaTeam *lastTeam[2];
    /**
     * @brief state of availableLayerMaps
     */
    std::vector<int> layerMapLockState;
    /**
     *  @brief state of each availableLayer in each map
     */
    std::vector<int> layerLockState;
};

} // namespace rota
