#ifndef TRANSIT_H
#define TRANSIT_H

#include <time.h>

#include "constellations.h"

typedef struct {
    time_t transit_utc;
    double max_altitude_deg;
    const char *visibility;
} transit_result_t;

void compute_transit(double lat, double lng, const constellation_t *c, time_t now, transit_result_t *out);

#endif
