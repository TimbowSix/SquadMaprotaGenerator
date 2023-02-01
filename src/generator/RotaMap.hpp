/**
 * @file RotaMap
 * @brief Object representation of a Squad Map
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */
#pragma once

#include <boost/numeric/ublas/vector.hpp>
#include <iostream>
#include <map>
#include <string.h>
#include <vector>

#include "RotaLayer.hpp"
#include "RotaMode.hpp"

namespace rota {

class RotaMap {
  private:
    std::string name;
    std::vector<RotaLayer *> layers;
    std::vector<RotaMode *> modes;
    /**
     * @brief maps the modes of this rotaMap to a list of layers
     */
    std::map<RotaMode *, std::vector<RotaLayer *>> modeToLayers;
    /**
     * @brief biom properties of this maps
     */
    boost::numeric::ublas::vector<float> biomValues;

    /**
     * @brief a weight for this map based of layer votes
     *        gets calculated by calcMapVoteWeight
     *     	 and  should changes when a layer gets locked
     */
    std::map<RotaMode *, float> mapVoteWeights;

    /**
     * @brief map weights from optimizer
     */
    std::map<RotaMode *, float> mapWeights;

    std::map<RotaMap *, float> distances;
    std::vector<RotaMap *> neighbor;

    int lockTime;
    int currentLockTime;

    /**
     * @brief values for the sigmoid to calc. weights
     *
     *        0: mapVoteSlope, 1: mapVoteShift, 2: layerVoteSlope,
     *        3: layerVoteShift
     */
    float sigmoidValues[4];

    /**
     * @brief a counter to keep track of available layers
     */
    std::map<RotaMode *, int> availableLayers;

    /**
     * @brief a pointer from generator for maintaining own count
     *        the first number is the count of maps which are available through
     *        locks the second number is the count of maps with have available
     *        layer
     */
    std::map<RotaMode *, int> *availableLayerMaps;

  public:
    RotaMap(std::string name, std::vector<float> biomValues, int lockTime,
            std::map<RotaMode *, int> *availableLayerMaps);
    ~RotaMap(){};

    /**
     * @brief add a Layer to this map
     *        populates the modeToLayer map
     *        NOTE: initialize the mapVoteWeight for this layer Mode to 1
     *
     * @param layer type RotaLayer
     */
    void addLayer(RotaLayer *layer);

    /**
     * @brief deceases the current lock time of this map by 1
     */
    void decreaseLockTime();

    /**
     * @brief locks this Map and its neighbors for the defined lock time
     *
     * @param lockNeighbors whether neighbors should also be locked.
     */
    void lock();

    /**
     * @brief locks this map for the defined locktime.
     *
     * @param lockNeighbors whether neighbors should also be also locked for the
     * same amount of time
     */
    void lock(bool lockNeighbors);

    /**
     * @brief locks map for given amount of rounds.
     *        Use for custom locktime.
     *
     * @param locktime amount of rounds to lock
     */
    void lock(int locktime);

    /**
     * @brief locks map for given amount of rounds.
     *        Use for custom locktime.
     *        does not overwrite if locktime < existing locktime
     *
     *
     * @param locktime amount of rounds to lock
     * @param lockNeighbors whether neighbors should also be locked for the same
     * amount of time
     */
    void lock(int locktime, bool lockNeighbors);

    /**
     * @brief sets new locktime regardless of current locktime
     */
    void overwriteLock(int locktime);

    /**
     * @brief resets current locktime to 0
     */
    void unlock();

    /**
     * @brief calculates a mode specific weight for this map
     *        based on the weighted mean of layer votes
     *
     * @param mode
     */
    void calcMapVoteWeight(RotaMode *mode);

    /**
     * @brief calculates all weights for this map
     *        weight a specific for one mode and
     *        based on the weighted mean of layer votes
     */
    void calcAllMapVoteWeights();

    /**
     * @brief calculates all weight for all layers of this map
     */
    void calcLayerVoteWeights();

    /**
     * @brief checks if unlocked layers for given mode are available
     *
     * @param mode mode to check
     */
    bool hasLayersAvailable(RotaMode *mode);

    /**
     * @brief checks if mode exists for this map. regardless if the layers are
     * locked or not
     *
     * @param mode mode to check
     */
    bool hasMode(RotaMode *mode);

    // getter & setter
    void setLockTime(int lockTime);
    std::vector<RotaLayer *> *getLayer();
    std::string getName();
    std::map<RotaMode *, std::vector<RotaLayer *>> *getModeToLayers();
    bool isLocked();
    float getMapVoteWeight(RotaMode *mode);
    void setMapVoteWeight(RotaMode *mode, float weight);
    float getMapWeight(RotaMode *mode);
    void setMapWeight(RotaMode *mode, float weight);
    std::vector<RotaMode *> *getModes();

    void increaseAvailableLayers(RotaMode *mode);
    void decreaseAvailableLayers(RotaMode *mode);

    boost::numeric::ublas::vector<float> *getBiomValues();
    void addNeighbour(RotaMap *map);
    std::vector<RotaMap *> *getNeighbor();
    int getCurrLockTime();
    std::map<RotaMode *, int> *getAvailableLayers();

    void setSigmoidValues(float mapVoteSlope, float mapVoteShift,
                          float layerVoteSlope, float layerVoteShift);
};
} // namespace rota
