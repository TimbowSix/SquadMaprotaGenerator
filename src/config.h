#ifndef rota_config_h
#define rota_config_h

#define LAYER_PATH "./data/layers.json"
#define BIOMS_PATH "./data/bioms.json"
#define CURRENT_MAP_DIST_PATH "./data/current_map_dist.json"
#define WEIGHT_PARAMS_PATH "./data/weight_params.json"
#define SAVE_PATH "./data/save.json"
#define CONFIG_PATH "./config.json"
#define MAPS_OVERWRITE_PATH "./data/maps_overwrite.json"
#define MODE_DIST_OVERWRITE_PATH "./data/mode_distribution_overwrite.json"

typedef struct gameMode gameMode;
struct gameMode
{
    char *name;
    double probability;
};

typedef struct poolDistribution poolDistribution;
struct poolDistribution
{
    char *poolName;
    double probability;
};

typedef struct modePool modePool;
struct modePool
{
    char *name;
    gameMode *gameMods;
};

typedef struct modeDistribution modeDistribution;
struct modeDistribution
{
    modePool *modePools;
    poolDistribution *poolDist;
    int poolSpacing;
    int spaceMain;
};

typedef struct rotaConfig rotaConfig;
struct rotaConfig
{
    int numberOfRotas;
    int numberOfLayers;
    int seedLayer;
    char *outputPath;
    char *layerVoteApiUrl;
    modeDistribution *modeDist;
    char *maps; // list of mapsnames
    int biomSpacing;
    int layerLockTime;
    double minBiomDistance;
    double mapVoteSlope;
    double mapVoteShift;
    double layerVoteSlope;
    double layerVoteShift;
    int useVoteWeight;
    int useMapWeight;
    int saveExpectedMapDist;
    int useLockTimeModifier;
    int autoOptimize;
    int fixUnavailable;
    int debug;
};

/**
 * reads config.json and returns a pointer to a rotaConfig struct
 */
rotaConfig *createConfig();

/**
 * free config mem
 */
void delConfig(rotaConfig *config);

#endif