/**
 * @file RotaLayer
 * @brief Object representation of a Squad Layer
 *
 * @author tim3 (timbow)
 * @author tim1 (fletschoa)
 * @author Kay  (kayms)
 */
#pragma once

#include <iostream>
#include <string>

#include "RotaMode.hpp"
#include "RotaTeam.hpp"

namespace rota {
class RotaMap;

class RotaLayer {
  private:
    std::string name;

    /**
     * @brief mode of this layer
     */
    RotaMode *mode;

    /**
     * @brief map of this layer
     */
    RotaMap *map;

    /**
     * @brief teams of this layer
     *        index 0 -> team 1
     *        index 1 -> team 2
     */
    RotaTeam *teams[2];

    /**
     * @brief votes given from extern source
     */
    float votes;

    /**
     * @brief weight for a weighed choice of layers
     *        gets calculated by setVoteWeight
     */
    float voteWeight;

    /**
     * @brief default time for locking a layer
     */
    int lockTime;

    /**
     * @brief time with has to pass to unlock this layer
     */
    int currLockTime;

  public:
    RotaLayer(std::string name, float votes);
    //~RotaLayer();
    /**
     * @brief lockes this layer for a time
     *        if a layer is locked it cannot be drawn
     *
     * @param time locktime
     */
    void lock(unsigned int time);

    /**
     * @brief lockes this layer for the defined lock time
     */
    void lock();

    /**
     * @brief decrease the current locktime by 1
     */
    void decreaseLockTime();

    // getter & setter
    float getVotes();
    RotaTeam *getTeam(int index);
    RotaMode *getMode();

    void setTeam(RotaTeam *team, int index);
    void setMode(RotaMode *mode);

    std::string getName();
    bool isLocked();
    float getVoteWeight();
    void setVoteWeight(float slope, float shift);
};
} // namespace rota
