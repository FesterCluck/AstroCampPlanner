#ifndef PARK_H
#define PARK_H

#include <stddef.h>
#include <stdint.h>

#define PARK_NAME_LEN 64
#define PARK_STATE_LEN 4
#define PARK_RECORD_SIZE (4 + PARK_NAME_LEN + PARK_STATE_LEN + 8 + 8 + 4 + 4)
#define PARK_MAX_COUNT 32

typedef struct {
    uint32_t id;
    char name[PARK_NAME_LEN];
    char state[PARK_STATE_LEN];
    double latitude;
    double longitude;
    uint32_t established_year;
    uint32_t area_acres;
} park_t;

extern const park_t PARKS[];
extern const size_t PARK_COUNT;

const park_t *park_find_by_name(const char *name);

const park_t *park_find_nearest(double lat, double lon, double *out_distance_miles);

double haversine_miles(double lat1, double lon1, double lat2, double lon2);

void park_serialize(const park_t *p, unsigned char *out);
void park_deserialize(const unsigned char *in, park_t *out);

void pack_double_be(unsigned char *out, double v);
double unpack_double_be(const unsigned char *in);

#endif
