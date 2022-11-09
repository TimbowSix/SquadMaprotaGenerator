#include <stdio.h>

#include "config.h"
#include "io.h"

rotaConfig *createConfig()
{
    struct json_object *jConfig;
    readJsonFile(CONFIG_PATH, &jConfig);

    rotaConfig *config = malloc(sizeof(rotaConfig));
}

void delConfig(rotaConfig *config)
{
    free(config);
}