#include "constellations.h"

#include <strings.h>

const constellation_t CONSTELLATIONS[] = {
    {"Orion",        5.242,  -8.202},
    {"Ursa Major",   11.062, 61.751},
    {"Ursa Minor",   2.530,  89.264},
    {"Cassiopeia",   0.675,  56.537},
    {"Scorpius",     16.490, -26.432},
    {"Sagittarius",  18.403, -34.384},
    {"Leo",          10.139, 11.967},
    {"Taurus",       4.599,  16.509},
    {"Gemini",       7.755,  28.026},
    {"Cygnus",       20.690, 45.280},
    {"Lyra",         18.615, 38.784},
    {"Aquila",       19.846, 8.868},
    {"Crux",         12.443, -63.099},
    {"Canis Major",  6.752,  -16.716},
    {"Pegasus",      23.079, 15.205},
};
const size_t CONSTELLATION_COUNT = sizeof(CONSTELLATIONS) / sizeof(CONSTELLATIONS[0]);

const constellation_t *constellation_find(const char *name) {
    for (size_t i = 0; i < CONSTELLATION_COUNT; i++) {
        if (strcasecmp(CONSTELLATIONS[i].name, name) == 0) return &CONSTELLATIONS[i];
    }
    return NULL;
}
