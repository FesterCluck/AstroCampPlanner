#ifndef CONSTELLATIONS_H
#define CONSTELLATIONS_H

#include <stddef.h>

typedef struct {
    const char *name;
    double ra_hours;
    double dec_deg;
} constellation_t;

extern const constellation_t CONSTELLATIONS[];
extern const size_t CONSTELLATION_COUNT;

const constellation_t *constellation_find(const char *name);

#endif
