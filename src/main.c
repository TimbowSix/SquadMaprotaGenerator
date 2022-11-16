#include <stdio.h>
#include "config.h"
#include "data.h"
#include "io.h"
#include "biom.h"
#include <json.h>

int main()
{
    struct json_object *object;
    // char *url = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req";
    //   readJsonFile("../config.json", &object);

    // writeJsonFile("../testConfig.json", object);
    // printf("%s", json_object_to_json_string(object));

    ioInit();

    rotaConfig *config = createConfig();
    // delConfig(config);
    // biom *bioms;
    // int biomLen = getBioms(&bioms);
    // printBioms(bioms, biomLen);
    // delBioms(bioms, biomLen);

    rotaLayer *layers;
    getLayers(config, &layers);

    // getLayerData(url, &object);
    // printf("%s", json_object_to_json_string(object));
}