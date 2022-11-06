#include <stdio.h>
#include "config.h"
#include "data.h"
#include "io.h"
#include <json.h>

int main(char **args)
{
    struct json_object *object;
    char *url = "https://welovesquad.com/wp-admin/admin-ajax.php?action=getLayerVotes_req";
    //  readJsonFile("../config.json", &object);

    // writeJsonFile("../testConfig.json", object);
    // printf("%s", json_object_to_json_string(object));

    ioInit();

    getLayerData(url, &object);
    printf("%s", json_object_to_json_string(object));
}