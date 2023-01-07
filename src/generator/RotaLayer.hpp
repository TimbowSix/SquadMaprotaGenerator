#pragma once

#include <iostream>


#include "RotaMode.hpp"
#include "RotaTeam.hpp"



namespace rota
{
    class RotaMap;

    class RotaLayer
    {
    private:
        char *name;
        RotaMode *mode;
        RotaMap *map;
        float votes;
        RotaTeam *teams[2];
        float voteWeight;
        int lockTime;
        int currLockTime;

    public:
        RotaLayer();
        ~RotaLayer();
        void lock();
        void decreaseLockTime();
    };
}