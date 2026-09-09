#include "transit.h"

#include <math.h>
#include <stdlib.h>

static double mod24(double x) {
    double m = fmod(x, 24.0);
    if (m < 0) m += 24.0;
    return m;
}

static double julian_date_0h(int year, int month, int day) {
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    int a = year / 100;
    int b = 2 - a + a / 4;
    return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + b - 1524.5;
}

static double gmst0_hours(int year, int month, int day) {
    double jd0 = julian_date_0h(year, month, day);
    double t = (jd0 - 2451545.0) / 36525.0;
    double gmst0 = 6.697374558 + 2400.051336 * t + 0.000025862 * t * t;
    return mod24(gmst0);
}

static double transit_ut_hours(int year, int month, int day, double lng_deg, double ra_hours) {
    double gmst0 = gmst0_hours(year, month, day);
    double diff = mod24(ra_hours - lng_deg / 15.0 - gmst0);
    return diff / 1.0027379093;
}

void compute_transit(double lat, double lng, const constellation_t *c, time_t now, transit_result_t *out) {
    for (int day_offset = 0; day_offset < 2; day_offset++) {
        time_t candidate_date = now + (time_t)day_offset * 86400;
        struct tm date_tm;
        gmtime_r(&candidate_date, &date_tm);
        int year = date_tm.tm_year + 1900, month = date_tm.tm_mon + 1, day = date_tm.tm_mday;

        double ut_hours = transit_ut_hours(year, month, day, lng, c->ra_hours);

        struct tm midnight_tm;
        gmtime_r(&candidate_date, &midnight_tm);
        midnight_tm.tm_hour = 0;
        midnight_tm.tm_min = 0;
        midnight_tm.tm_sec = 0;
        time_t midnight = timegm(&midnight_tm);
        time_t transit = midnight + (time_t)llround(ut_hours * 3600.0);

        if (transit > now) {
            out->transit_utc = transit;
            break;
        }
        out->transit_utc = transit;
    }

    double circumpolar_test = lat + c->dec_deg;
    double never_rises_test = lat - c->dec_deg;
    if (fabs(circumpolar_test) >= 90.0) {
        out->visibility = "circumpolar";
    } else if (fabs(never_rises_test) >= 90.0) {
        out->visibility = "never_rises";
    } else {
        out->visibility = "rises_and_sets";
    }
    out->max_altitude_deg = 90.0 - fabs(lat - c->dec_deg);
}
