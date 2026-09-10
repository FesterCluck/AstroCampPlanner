#include "park.h"

#include <arpa/inet.h>
#include <endian.h>
#include <math.h>
#include <string.h>
#include <strings.h>

const park_t PARKS[] = {
    {1,  "Yellowstone",             "WY", 44.4280, -110.5885, 1872, 2219791},
    {2,  "Yosemite",                "CA", 37.8651, -119.5383, 1890, 761268},
    {3,  "Grand Canyon",            "AZ", 36.1069, -112.1129, 1919, 1201647},
    {4,  "Zion",                    "UT", 37.2982, -113.0263, 1919, 147242},
    {5,  "Acadia",                  "ME", 44.3386, -68.2733,  1919, 49076},
    {6,  "Everglades",              "FL", 25.2866, -80.8987,  1934, 1508538},
    {7,  "Denali",                  "AK", 63.1148, -151.1926, 1917, 6045153},
    {8,  "Great Smoky Mountains",   "TN", 35.6118, -83.4895,  1934, 522419},
    {9,  "Olympic",                 "WA", 47.8021, -123.6044, 1938, 922650},
    {10, "Joshua Tree",             "CA", 33.8734, -115.9010, 1994, 795156},
};
const size_t PARK_COUNT = sizeof(PARKS) / sizeof(PARKS[0]);

const park_t *park_find_by_name(const char *name) {
    for (size_t i = 0; i < PARK_COUNT; i++) {
        if (strcasecmp(PARKS[i].name, name) == 0) return &PARKS[i];
    }
    return NULL;
}

double haversine_miles(double lat1, double lon1, double lat2, double lon2) {
    const double earth_radius_miles = 3958.8;
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
                   sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return earth_radius_miles * c;
}

const park_t *park_find_nearest(double lat, double lon, double *out_distance_miles) {
    const park_t *best = &PARKS[0];
    double best_dist = haversine_miles(lat, lon, best->latitude, best->longitude);
    for (size_t i = 1; i < PARK_COUNT; i++) {
        double d = haversine_miles(lat, lon, PARKS[i].latitude, PARKS[i].longitude);
        if (d < best_dist) {
            best = &PARKS[i];
            best_dist = d;
        }
    }
    *out_distance_miles = best_dist;
    return best;
}

void pack_double_be(unsigned char *out, double v) {
    uint64_t bits;
    memcpy(&bits, &v, sizeof(bits));
    uint64_t be = htobe64(bits);
    memcpy(out, &be, sizeof(be));
}

double unpack_double_be(const unsigned char *in) {
    uint64_t be;
    memcpy(&be, in, sizeof(be));
    uint64_t bits = be64toh(be);
    double v;
    memcpy(&v, &bits, sizeof(v));
    return v;
}

void park_serialize(const park_t *p, unsigned char *out) {
    uint32_t id_be = htonl(p->id);
    memcpy(out, &id_be, 4);
    out += 4;

    memset(out, 0, PARK_NAME_LEN);
    memcpy(out, p->name, strnlen(p->name, PARK_NAME_LEN - 1));
    out += PARK_NAME_LEN;

    memset(out, 0, PARK_STATE_LEN);
    memcpy(out, p->state, strnlen(p->state, PARK_STATE_LEN - 1));
    out += PARK_STATE_LEN;

    pack_double_be(out, p->latitude);
    out += 8;
    pack_double_be(out, p->longitude);
    out += 8;

    uint32_t year_be = htonl(p->established_year);
    memcpy(out, &year_be, 4);
    out += 4;

    uint32_t acres_be = htonl(p->area_acres);
    memcpy(out, &acres_be, 4);
}

void park_deserialize(const unsigned char *in, park_t *out) {
    uint32_t id_be;
    memcpy(&id_be, in, 4);
    out->id = ntohl(id_be);
    in += 4;

    memcpy(out->name, in, PARK_NAME_LEN);
    out->name[PARK_NAME_LEN - 1] = '\0';
    in += PARK_NAME_LEN;

    memcpy(out->state, in, PARK_STATE_LEN);
    out->state[PARK_STATE_LEN - 1] = '\0';
    in += PARK_STATE_LEN;

    out->latitude = unpack_double_be(in);
    in += 8;
    out->longitude = unpack_double_be(in);
    in += 8;

    uint32_t year_be;
    memcpy(&year_be, in, 4);
    out->established_year = ntohl(year_be);
    in += 4;

    uint32_t acres_be;
    memcpy(&acres_be, in, 4);
    out->area_acres = ntohl(acres_be);
}
