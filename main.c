//api direcionada para transporte publico e treinamento de boas praticas
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cjson/cJSON.h>
#include<math.h>

#define MAX_ID_LEN 64
#define PI 3.14159265358979323846
#define EARTH_RADIUS_KM 6371.0

typedef struct {
    char id[MAX_ID_LEN];
    double latitude;
    double longitude;
} BusStop;

double to_rad(double deg) {
    return deg * PI / 180.0;
}

double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dlat = to_rad(lat2 - lat1);
    double dlon = to_rad(lon2 - lon1);

    double a = sin(dlat/2) * sin(dlat/2) +
               cos(to_rad(lat1)) * cos(to_rad(lat2)) *
               sin(dlon/2) * sin(dlon/2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return EARTH_RADIUS_KM * c;
}

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

    printf("=== Distâncias entre paradas ===\n\n");

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            double dist = haversine(
                stops[i].latitude,  stops[i].longitude,
                stops[j].latitude,  stops[j].longitude
            );
            printf("Parada %d → Parada %d : %.3f km\n", i+1, j+1, dist);
        }
    }
}