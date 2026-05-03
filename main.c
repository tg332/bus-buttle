//api direcionada para transporte publico e treinamento de boas praticas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#define MAX_ID_LEN 64

typedef struct {
    char id[MAX_ID_LEN];
    double latitude;
    double longitude;
} BusStop;

// Lê o arquivo inteiro em memória
char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Erro ao abrir %s\n", path); return NULL; }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

int main() {
    char *json = read_file("export.geojson");
    if (!json) return 1;

    cJSON *root     = cJSON_Parse(json);
    cJSON *features = cJSON_GetObjectItem(root, "features");

    int total = cJSON_GetArraySize(features);
    BusStop *stops = malloc(total * sizeof(BusStop));
    int count = 0;

    cJSON *feature;
    cJSON_ArrayForEach(feature, features) {
        cJSON *props    = cJSON_GetObjectItem(feature, "properties");
        cJSON *geometry = cJSON_GetObjectItem(feature, "geometry");
        cJSON *coords   = cJSON_GetObjectItem(geometry, "coordinates");

        // pega o ID
        cJSON *id = cJSON_GetObjectItem(props, "@id");
        strncpy(stops[count].id, id->valuestring, MAX_ID_LEN - 1);

        // pega as coordenadas [lon, lat]
        stops[count].longitude = cJSON_GetArrayItem(coords, 0)->valuedouble;
        stops[count].latitude  = cJSON_GetArrayItem(coords, 1)->valuedouble;

        count++;
    }

    printf("=== Paradas de Breu Branco ===\n\n");
    for (int i = 0; i < count; i++) {
        printf("Parada %d\n", i + 1);
        printf("  ID  : %s\n", stops[i].id);
        printf("  Lat : %.7f\n", stops[i].latitude);
        printf("  Lon : %.7f\n\n", stops[i].longitude);
    }
    printf("Total: %d paradas\n", count);

    free(stops);
    free(json);
    cJSON_Delete(root);
    return 0;
}