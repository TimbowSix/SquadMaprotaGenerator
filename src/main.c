#include <stdio.h>
#include <json.h>

#include "config.h"
#include "data.h"
#include "io.h"
#include "biom.h"
#include "data.h"

int main()
{
    rotaMap *maps;
    rotaLayer *layers;
    rotaMode *modes;
    struct json_object *object;
    // char *url = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req";
    //   readJsonFile("../config.json", &object);

    // writeJsonFile("../testConfig.json", object);
    // printf("%s", json_object_to_json_string(object));

    ioInit();

    rotaConfig *config = createConfig();
    initialize(config, &maps, &layers, &modes);
    printf("fertig");
}