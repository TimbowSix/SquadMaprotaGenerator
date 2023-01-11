/**
 * @file RotaMap
 * @brief Object representation of a Squad Map
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */
#pragma once

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

    /**
     * @brief maps the modes of this rotaMap to a list of layers
     */
    std::map<RotaMode *, std::vector<RotaLayer *>> modeToLayers;
    /**
     * @brief biom properties of this maps
     */
    std::vector<float> biomValues;

    /**
     * @brief a weight for this map based of layer votes
     *        gets calculated by calcMapVoteWeight
     *     	 and  should changes when a layer gets locked
     */
    std::map<RotaMode *, float> mapVoteWeights;

    /**
     * @brief a helper sum for calculating the actual map weight
     *        should change if a layer gets locked
     */
    std::map<RotaMode *, float> mapVoteWeightSum;

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

  public:
    RotaMap(std::string name, std::vector<float> biomValues, int lockTime);
    ~RotaMap(){};
    void addLayer(RotaLayer *layer);

    /**
     * @brief deceases the current lock time of this map by 1
     */
    void deceaseLockTime();
    /**
     * @brief resets the current back to default lock time
     */
    void resetLockTime();

    /**
     * @brief locks map for given amount of rounds.
     *        Use for custom locktime.
     *        For normal lock use resetLockTime
     *
     * @param locktime amount of rounds to lock
    */
    void lock(int locktime);

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
     * @brief calculates the map weight for a mode based on the map Vote weights
     *
     * @param mode
     * @param params values which a found bei the optimizer
     * @returns the map weight for given mode
     */
    float calcMapWeight(RotaMode *mode);

    /**
     * @brief calculates all weight for all layers of this map
     */
    void calcLayerVoteWeights();

    /**
     * @brief calculates a new map vote weight for this mode
     *        must be called wenn a layer gets locked
     *        updates the mapWeightSum for a fast map weight
     *        calculation
     */
    void calcNewMapVoteWeight(RotaMode *mode);

    // getter & setter
    void setLockTime(int lockTime);
    std::vector<RotaLayer *> *getLayer();
    std::string getName();
    std::map<RotaMode *, std::vector<RotaLayer *>> *getModeToLayers();
    int getLocktime();

};
} // namespace rota
