#include "parks.h"

bool_t
xdr_Park (XDR *xdrs, Park *objp)
{
	register int32_t *buf;

	 if (!xdr_u_int (xdrs, &objp->id))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->name, 64))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->state, 4))
		 return FALSE;
	 if (!xdr_double (xdrs, &objp->latitude))
		 return FALSE;
	 if (!xdr_double (xdrs, &objp->longitude))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->established_year))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->area_acres))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_FindNearestArgs (XDR *xdrs, FindNearestArgs *objp)
{
	register int32_t *buf;

	 if (!xdr_double (xdrs, &objp->latitude))
		 return FALSE;
	 if (!xdr_double (xdrs, &objp->longitude))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_FindNearestResult (XDR *xdrs, FindNearestResult *objp)
{
	register int32_t *buf;

	 if (!xdr_Park (xdrs, &objp->park))
		 return FALSE;
	 if (!xdr_double (xdrs, &objp->distance_miles))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_ParkList (XDR *xdrs, ParkList *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->parks.parks_val, (u_int *) &objp->parks.parks_len, ~0,
		sizeof (Park), (xdrproc_t) xdr_Park))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_GetByNameResult (XDR *xdrs, GetByNameResult *objp)
{
	register int32_t *buf;

	 if (!xdr_bool (xdrs, &objp->found))
		 return FALSE;
	switch (objp->found) {
	case TRUE:
		 if (!xdr_Park (xdrs, &objp->GetByNameResult_u.park))
			 return FALSE;
		break;
	case FALSE:
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
