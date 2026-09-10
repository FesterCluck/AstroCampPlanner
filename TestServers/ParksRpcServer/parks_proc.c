#include "park.h"
#include "parks.h"

#include <stdio.h>
#include <string.h>

static void to_rpc_park(const park_t *src, Park *dst, char *name_buf, char *state_buf) {
    dst->id = src->id;
    snprintf(name_buf, PARK_NAME_LEN, "%s", src->name);
    snprintf(state_buf, PARK_STATE_LEN, "%s", src->state);
    dst->name = name_buf;
    dst->state = state_buf;
    dst->latitude = src->latitude;
    dst->longitude = src->longitude;
    dst->established_year = src->established_year;
    dst->area_acres = src->area_acres;
}

GetByNameResult *
getbyname_1_svc(char **argp, struct svc_req *rqstp)
{
    (void)rqstp;
    static GetByNameResult result;
    static char name_buf[PARK_NAME_LEN];
    static char state_buf[PARK_STATE_LEN];

    memset(&result, 0, sizeof(result));
    const park_t *p = park_find_by_name(*argp);
    if (!p) {
        result.found = FALSE;
        return &result;
    }
    result.found = TRUE;
    to_rpc_park(p, &result.GetByNameResult_u.park, name_buf, state_buf);
    return &result;
}

FindNearestResult *
findnearest_1_svc(FindNearestArgs *argp, struct svc_req *rqstp)
{
    (void)rqstp;
    static FindNearestResult result;
    static char name_buf[PARK_NAME_LEN];
    static char state_buf[PARK_STATE_LEN];

    double distance;
    const park_t *p = park_find_nearest(argp->latitude, argp->longitude, &distance);
    to_rpc_park(p, &result.park, name_buf, state_buf);
    result.distance_miles = distance;
    return &result;
}

ParkList *
listall_1_svc(void *argp, struct svc_req *rqstp)
{
    (void)argp;
    (void)rqstp;
    static ParkList result;
    static Park parks[PARK_MAX_COUNT];
    static char name_bufs[PARK_MAX_COUNT][PARK_NAME_LEN];
    static char state_bufs[PARK_MAX_COUNT][PARK_STATE_LEN];

    u_int n = (u_int)(PARK_COUNT < PARK_MAX_COUNT ? PARK_COUNT : PARK_MAX_COUNT);
    for (u_int i = 0; i < n; i++) {
        to_rpc_park(&PARKS[i], &parks[i], name_bufs[i], state_bufs[i]);
    }
    result.parks.parks_len = n;
    result.parks.parks_val = parks;
    return &result;
}

int
parks_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
    (void)transp;
    (void)xdr_result;
    (void)result;
    return 1;
}
