#include <stdio.h>
#include <json.h>
#include <time.h>

#include "config.h"
#include "data.h"
#include "io.h"
#include "biom.h"
#include "data.h"
#include "utils.h"
#include "generator.h"

int main()
{
    rotaMap *maps;
    rotaLayer *layers;
    rotaMode *modes;
    mapRota *rota;
    int mapsLen;
    int layerLen;
    int modeLen;

    struct json_object *object;
    // char *url = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req";
    //   readJsonFile("../config.json", &object);

    // writeJsonFile("../testConfig.json", object);
    // printf("%s", json_object_to_json_string(object));

    ioInit();
    unsigned int seed = (unsigned)time(0);
    printf("Seed: %u\n", seed);
    initRandom(seed);

    rotaConfig *config = createConfig();
    initialize(config, &maps, &layers, &modes, &mapsLen, &layerLen, &modeLen);

    generateRota(config, &rota, maps, mapsLen, modes, modeLen);
    printf("Index \r\t Mode \r\t\t\t Map \r\t\t\t\t\t Layer\n");
    for (int i = 999980; i < 1000000; i++)
    {
        printf("%i \r\t %s \r\t\t\t %s \r\t\t\t\t\t %s\n", i, rota->modes[i]->name, rota->maps[i]->name, rota->rotation[i]->name);
    }
}