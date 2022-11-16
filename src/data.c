#include <stdio.h>
#include <json.h>
#include <errno.h>

#include "data.h"
#include "io.h"

void initializeMaps(rotaConfig *conf, rotaMap *maps)
{
}

void getDist(rotaMap *maps, double **distances)
{
}

void normalizeBiomMapSize(double **bioms)
{
}

void getLayers(rotaConfig *conf, rotaLayer **allLayers)
{
    if (conf->updateLayer == 1)
    {
        json_object *jLayers;
        getLayerData(conf->layerVoteApiUrl, &jLayers);

        json_object *layerNames;
        json_object *mapVotes;

        layerNames = json_object_object_get(jLayers, "AxisLabels");
        int len = json_object_array_length(layerNames);
        rotaLayer *layers = malloc(len * sizeof(rotaLayer));

        // votes & status
        json_object *arr = json_object_object_get(jLayers, "DataSet");
        json_object *layerStatus = json_object_array_get_idx(arr, 2);
        json_object *upArr = json_object_array_get_idx(arr, 0);
        json_object *downArr = json_object_array_get_idx(arr, 1);

        if (json_object_array_length(layerNames) != json_object_array_length(upArr) || json_object_array_length(layerNames) != json_object_array_length(downArr) || json_object_array_length(layerNames) != json_object_array_length(layerStatus))
        {
            printf("Error while getting Layers: layer length dont match to attributes");
            exit(EXIT_FAILURE);
        }

        int k = 0;
        for (int i = 0; i < len; i++)
        {
            if (json_object_get_int(json_object_array_get_idx(layerStatus, i)) == 0)
            {
                const char *n = json_object_get_string(json_object_array_get_idx(layerNames, i));
                if (strlen(n) > MAX_LAYER_NAME_LENGTH)
                {
                    printf("a layer name cannot be over %i chars", MAX_LAYER_NAME_LENGTH);
                    exit(EXIT_FAILURE);
                }
                strcpy(layers[k].name, n);
                layers[k].votes = (double)(json_object_get_int(json_object_array_get_idx(upArr, i)) + json_object_get_int(json_object_array_get_idx(downArr, i)));
                k++;
            }
        }

        // write file
        saveLayers(layers, len);
    }
    else
    {
        // read from file
        int layerCount = loadLayers(allLayers);
        if (layerCount < 0)
        {
            printf("ERROR while loading, cannot continue\n");
            exit(EXIT_FAILURE);
        }
    }
}

int saveLayers(rotaLayer *layers, int len)
{
    FILE *file;

    file = fopen(LAYER_PATH, "w");
    if (file == NULL)
    {
        printf("ERROR can't save layer file: %s", strerror(errno));
        return 0;
    }
    rotaLayerFileHeader fileHeader = {.count = len};
    fwrite(&fileHeader, sizeof(rotaLayerFileHeader), 1, file);
    for (int i = 0; i < len; i++)
    {
        fwrite(&layers[i], sizeof(rotaLayer), 1, file);
    }
    fclose(file);
    return 1;
}

int loadLayers(rotaLayer **layers)
{
    FILE *file;
    file = fopen(LAYER_PATH, "r");
    if (file == NULL)
    {
        printf("ERROR loading layer file %s", strerror(errno));
        return -1;
    }

    rotaLayerFileHeader fileHeader;
    fread(&fileHeader, sizeof(rotaLayerFileHeader), 1, file);
    (*layers) = malloc(fileHeader.count * sizeof(rotaLayer));

    int i = 0;
    rotaLayer layer;
    while (fread(&((*layers)[i]), sizeof(rotaLayer), 1, file))
    {
        i++;
    }
    fclose(file);
    return i;
}

void fixUnavailable(rotaConfig *conf, rotaMap *maps)
{
}

void buildConfig(rotaConfig *con)
{
}

int checkChanges()
{
}