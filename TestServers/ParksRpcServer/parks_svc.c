#include "parks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

static void
parks_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		char *getbyname_1_arg;
		FindNearestArgs findnearest_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case GETBYNAME:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_GetByNameResult;
		local = (char *(*)(char *, struct svc_req *)) getbyname_1_svc;
		break;

	case FINDNEAREST:
		_xdr_argument = (xdrproc_t) xdr_FindNearestArgs;
		_xdr_result = (xdrproc_t) xdr_FindNearestResult;
		local = (char *(*)(char *, struct svc_req *)) findnearest_1_svc;
		break;

	case LISTALL:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_ParkList;
		local = (char *(*)(char *, struct svc_req *)) listall_1_svc;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	return;
}

int
main (int argc, char **argv)
{
	register SVCXPRT *transp;
	int port = 9090;
	int sock;
	int yes = 1;
	struct sockaddr_in addr;

	if (argc > 1) port = atoi(argv[1]);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((unsigned short)port);
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		perror("bind");
		exit(1);
	}
	if (listen(sock, SOMAXCONN) != 0) {
		perror("listen");
		exit(1);
	}

	transp = svctcp_create(sock, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, PARKS_PROG, PARKS_VERS, parks_prog_1, 0)) {
		fprintf (stderr, "%s", "unable to register (PARKS_PROG, PARKS_VERS, tcp).");
		exit(1);
	}

	printf("faux National Parks RPC service (ONC RPC/TCP) listening on 127.0.0.1:%d "
	       "(prog 0x%x vers %d)\n", port, PARKS_PROG, PARKS_VERS);
	fflush(stdout);

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
}
