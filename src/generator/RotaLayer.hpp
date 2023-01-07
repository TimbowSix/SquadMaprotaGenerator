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



namespace rota
{
    class RotaMap;

    class RotaLayer
    {
    private:
        std::string name;
        RotaMode *mode;
        RotaMap *map;
        float votes;
        RotaTeam *teams[2];
        float voteWeight;
        int lockTime;
        int currLockTime;

    public:
        RotaLayer(std::string name, float votes);
        //~RotaLayer();
        void lock();
        void decreaseLockTime();


        std::string getName();
        float getVotes();
        RotaTeam* getTeam(int index);
        RotaMode* getMode();

        void setTeam(RotaTeam *team, int index);
        void setMode(RotaMode *mode);
    };
}
