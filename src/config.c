#include <stdio.h>

#include "config.h"

rotaConfig *createConfig()
{
    struct json_object *jConfig;
    if (readJsonFile(CONFIG_PATH, &jConfig) == 0)
    {
        exit(EXIT_FAILURE);
    }
    rotaConfig *config = malloc(sizeof(rotaConfig));
    config->outputPath = malloc(DEFAULT_CONFIG_STRING_LENGTH * sizeof(char));
    config->layerVoteApiUrl = malloc(DEFAULT_CONFIG_STRING_LENGTH * sizeof(char));

    int numberOfRotas = 0;

    getJsonIntValue(jConfig, "number_of_rotas", &config->numberOfRotas);
    getJsonIntValue(jConfig, "number_of_layers", &config->numberOfLayers);
    getJsonIntValue(jConfig, "seed_layer", &config->seedLayer);
    getJsonBoolValue(jConfig, "update_layers", &config->updateLayer);
    getJsonStringValue(jConfig, "output_path", &config->outputPath, DEFAULT_CONFIG_STRING_LENGTH);
    getJsonStringValue(jConfig, "layer_vote_api_url", &config->layerVoteApiUrl, DEFAULT_CONFIG_STRING_LENGTH);
    getJsonIntValue(jConfig, "biom_spacing", &config->biomSpacing);
    getJsonIntValue(jConfig, "layer_lockTime", &config->layerLockTime);
    getJsonDoubleValue(jConfig, "min_biom_distance", &config->minBiomDistance);
    getJsonDoubleValue(jConfig, "mapVote_slope", &config->mapVoteSlope);
    getJsonDoubleValue(jConfig, "mapVote_shift", &config->mapVoteShift);
    getJsonDoubleValue(jConfig, "layerVote_slope", &config->layerVoteSlope);
    getJsonDoubleValue(jConfig, "layerVote_shift", &config->layerVoteShift);
    getJsonBoolValue(jConfig, "use_vote_weight", &config->useVoteWeight);
    getJsonBoolValue(jConfig, "use_map_weight", &config->useMapWeight);
    getJsonBoolValue(jConfig, "save_expected_map_dist", &config->saveExpectedMapDist);
    getJsonBoolValue(jConfig, "use_lock_time_modifier", &config->useLockTimeModifier);
    getJsonBoolValue(jConfig, "auto_optimize", &config->autoOptimize);
    getJsonBoolValue(jConfig, "fix_unavailable", &config->fixUnavailable);
    getJsonBoolValue(jConfig, "debug", &config->debug);

    // config maps stuff
    struct json_object *jMaps;
    json_object_object_get_ex(jConfig, "maps", &jMaps);
    if (json_object_get_type(jMaps) != json_type_array)
    {
        printf("Error config.json maps is not an array\n");
        exit(EXIT_FAILURE);
    }
    size_t mapsLen = json_object_array_length(jMaps);
    if (mapsLen <= 0)
    {
        printf("Error config map entries are 0");
        exit(EXIT_FAILURE);
    }
    config->mapCount = mapsLen;

    config->maps = malloc(mapsLen * sizeof(void *));

    struct json_object *jMap;
    for (int i = 0; i < mapsLen; i++)
    {
        jMap = json_object_array_get_idx(jMaps, i);
        if (json_object_get_type(jMap) != json_type_string)
        {
            printf("Error in config: a map is not a string");
            exit(EXIT_FAILURE);
        }
        config->maps[i] = malloc(DEFAULT_CONFIG_STRING_LENGTH * sizeof(char));
        strcpy(config->maps[i], json_object_get_string(jMap));
    }

    // mode dist

    config->modeDist = malloc(sizeof(modeDistribution));

    struct json_object *modeDist;
    json_object_object_get_ex(jConfig, "mode_distribution", &modeDist);

    getJsonIntValue(modeDist, "pool_spacing", &config->modeDist->poolSpacing);
    getJsonBoolValue(modeDist, "space_main", &config->modeDist->spaceMain);

    struct json_object *pools;
    struct json_object *poolDist;

    json_object_object_get_ex(modeDist, "pools", &pools);
    json_object_object_get_ex(modeDist, "pool_distribution", &poolDist);

    int poolLen = json_object_object_length(pools);
    int poolDistLen = json_object_object_length(poolDist);

    // get space for pools and pool dist
    config->modeDist->modePools = malloc(poolLen * sizeof(modePool *));
    config->modeDist->poolDist = malloc(poolDistLen * sizeof(poolDistribution *));
    config->modeDist->poolCount = poolLen;
    config->modeDist->poolDistCount = poolDistLen;

    // get mode pool dist
    int i = 0;
    json_object_object_foreach(poolDist, key, val)
    {
        config->modeDist->poolDist[i] = malloc(sizeof(poolDistribution)); //@todo free after
        config->modeDist->poolDist[i]->poolName = malloc((strlen(key) + 1) * sizeof(char));
        strcpy(config->modeDist->poolDist[i]->poolName, key);
        if (json_object_get_type(val) != json_type_double)
        {
            printf("Error config: \"%s\" has no double value", key);
            exit(EXIT_FAILURE);
        }
        config->modeDist->poolDist[i]->probability = json_object_get_double(val);
        i++;
    }

    i = 0;
    json_object_object_foreach(pools, poolName, poolObj)
    {
        config->modeDist->modePools[i] = malloc(sizeof(modePool));
        config->modeDist->modePools[i]->name = malloc((strlen(poolName) + 1) * sizeof(char));
        strcpy(config->modeDist->modePools[i]->name, poolName);
        size_t modeCount = json_object_object_length(poolObj);
        config->modeDist->modePools[i]->poolCount = modeCount;
        config->modeDist->modePools[i]->gameMods = malloc(modeCount * sizeof(gameMode *));
        int j = 0;
        json_object_object_foreach(poolObj, modeName, modeProp)
        {
            config->modeDist->modePools[i]->gameMods[j] = malloc(sizeof(gameMode));
            config->modeDist->modePools[i]->gameMods[j]->name = malloc((strlen(modeName) + 1) * sizeof(char));
            strcpy(config->modeDist->modePools[i]->gameMods[j]->name, modeName);
            if (json_object_get_type(modeProp) != json_type_double)
            {
                printf("Error config: pools \"%s\" has no double value", modeName);
                exit(EXIT_FAILURE);
            }
            config->modeDist->modePools[i]->gameMods[j]->probability = json_object_get_double(modeProp);
            j++;
        }
        i++;
    }

    // check config

    if (config->modeDist->poolCount != config->modeDist->poolDistCount)
    {
        printf("Error config: pools count != pool dist count");
        exit(EXIT_FAILURE);
    }

    int found = 0;
    for (int i = 0; i < config->modeDist->poolDistCount; i++)
    {
        if (strcmp(config->modeDist->poolDist[i]->poolName, "main") == 0)
        {
            found = 1;
        }
    }
    if (found == 0)
    {
        printf("Error config: no main pool");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < config->modeDist->poolDistCount; i++)
    {
        found = 0;
        for (int j = 0; j < config->modeDist->poolCount; j++)
        {
            if (strcmp(config->modeDist->poolDist[i]->poolName, config->modeDist->modePools[j]->name) == 0)
            {
                found = 1;
                break;
            }
        }
        if (found == 0)
        {
            printf("Error config: entry \"%s\" in mode distribution cannot be found in pools", config->modeDist->poolDist[i]->poolName);
            exit(EXIT_FAILURE);
        }
    }

    double sum = 0;
    for (int i = 0; i < config->modeDist->poolCount; i++)
    {
        for (int j = 0; j < config->modeDist->modePools[i]->poolCount; j++)
        {
            sum += config->modeDist->modePools[i]->gameMods[j]->probability;
        }
        if (sum != 1)
        {
            printf("Error config: pool \"%s\" don't sum up to 1", config->modeDist->modePools[i]->name);
            exit(EXIT_FAILURE);
        }
        sum = 0;
    }

    sum = 0;
    for (int i = 0; i < config->modeDist->poolDistCount; i++)
    {
        sum += config->modeDist->poolDist[i]->probability;
    }
    if (sum != 1)
    {
        printf("Error config: pool distribution don't sum up to 1");
        exit(EXIT_FAILURE);
    }

    return config;
}

void delConfig(rotaConfig *config)
{
    free(config->outputPath);
    free(config->layerVoteApiUrl);
    for (int i = 0; i < config->mapCount; i++)
    {
        free(config->maps[i]);
    }
    free(config->maps);

    for (int i = 0; i < config->modeDist->poolDistCount; i++)
    {
        free(config->modeDist->poolDist[i]);
    }

    for (int i = 0; i < config->modeDist->poolCount; i++)
    {
        for (int j = 0; j < config->modeDist->modePools[i]->poolCount; j++)
        {
            free(config->modeDist->modePools[i]->gameMods[j]);
        }
        free(config->modeDist->modePools[i]);
    }

    free(config->modeDist->modePools);
    free(config->modeDist->poolDist);
    free(config->modeDist);
    free(config);
}
