#ifndef rota_config_h
#define rota_config_h

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
    int spaceMain;
};


struct rotaConfig {
    int numberOfRotas;
    int numberOfLayers;
    int seedLayer;
    char* outputPath;
    char* layerVoteApiUrl;
    struct modeDistribution* modeDists;
    char* maps; //list of mapsnames
    int biomSpacing;
    int layerLocktime;
    double minBiomDistance;
    double mapvoteSlope;
    double mapvoteShift;
    double layervoteSlope;
    double layervoteShift;
    int useVoteWeight;
    int useMapWeight;
    int saveExpectedMapDist;
    int useLockTimeModifier;
    int autoOptimize;
    int fixUnavailables;
    int debug;
};


#endif