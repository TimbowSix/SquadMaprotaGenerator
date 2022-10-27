#ifndef CONFIG
#define CONFIG

#include <stdbool.h>

struct gameMode{
    char* name;
    double probability;
};

struct poolDistribution{
    char* poolName;
    double probability;
};

struct modePool {
    char* name;
    struct gameMode* gameMods;
};

struct modeDistribution {
    struct modePool* modePools;
    struct poolDistribution* poolDists;
    int poolSpacing;
    bool spaceMain;
};


struct config {
    int numberOfRotas;
    int numberOfLayers;
    int seedLayer;
    char* outputPath;
    char* layerVoteApiUrl;
    struct modeDistribution* modeDists;
    enum maps; //list of mapsnames
    int biomSpacing;
    int layerLocktime;
    double minBiomDistance;
    double mapvoteSlope;
    double mapvoteShift;
    double layervoteSlope;
    double layervoteShift;
    bool useVoteWeight;
    bool useMapWeight;
    bool saveExpectedMapDist;
    bool useLockTimeModifier;
    bool autoOptimize;
    bool fixUnavailables;
    bool debug;
};


#endif