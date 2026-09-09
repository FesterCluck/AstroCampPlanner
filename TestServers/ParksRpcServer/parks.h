#ifndef _PARKS_H_RPCGEN
#define _PARKS_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct Park {
	u_int id;
	char *name;
	char *state;
	double latitude;
	double longitude;
	u_int established_year;
	u_int area_acres;
};
typedef struct Park Park;

struct FindNearestArgs {
	double latitude;
	double longitude;
};
typedef struct FindNearestArgs FindNearestArgs;

struct FindNearestResult {
	Park park;
	double distance_miles;
};
typedef struct FindNearestResult FindNearestResult;

struct ParkList {
	struct {
		u_int parks_len;
		Park *parks_val;
	} parks;
};
typedef struct ParkList ParkList;

struct GetByNameResult {
	bool_t found;
	union {
		Park park;
	} GetByNameResult_u;
};
typedef struct GetByNameResult GetByNameResult;

#define PARKS_PROG 0x20000001
#define PARKS_VERS 1

#if defined(__STDC__) || defined(__cplusplus)
#define GETBYNAME 1
extern  GetByNameResult * getbyname_1(char **, CLIENT *);
extern  GetByNameResult * getbyname_1_svc(char **, struct svc_req *);
#define FINDNEAREST 2
extern  FindNearestResult * findnearest_1(FindNearestArgs *, CLIENT *);
extern  FindNearestResult * findnearest_1_svc(FindNearestArgs *, struct svc_req *);
#define LISTALL 3
extern  ParkList * listall_1(void *, CLIENT *);
extern  ParkList * listall_1_svc(void *, struct svc_req *);
extern int parks_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else
#define GETBYNAME 1
extern  GetByNameResult * getbyname_1();
extern  GetByNameResult * getbyname_1_svc();
#define FINDNEAREST 2
extern  FindNearestResult * findnearest_1();
extern  FindNearestResult * findnearest_1_svc();
#define LISTALL 3
extern  ParkList * listall_1();
extern  ParkList * listall_1_svc();
extern int parks_prog_1_freeresult ();
#endif

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_Park (XDR *, Park*);
extern  bool_t xdr_FindNearestArgs (XDR *, FindNearestArgs*);
extern  bool_t xdr_FindNearestResult (XDR *, FindNearestResult*);
extern  bool_t xdr_ParkList (XDR *, ParkList*);
extern  bool_t xdr_GetByNameResult (XDR *, GetByNameResult*);

#else
extern bool_t xdr_Park ();
extern bool_t xdr_FindNearestArgs ();
extern bool_t xdr_FindNearestResult ();
extern bool_t xdr_ParkList ();
extern bool_t xdr_GetByNameResult ();

#endif

#ifdef __cplusplus
}
#endif

#endif
