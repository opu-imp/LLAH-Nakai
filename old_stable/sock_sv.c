#include "def_general.h"

#ifdef	LINUX

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sock_sv.h"

int InitSockSvTCP( u_short port, char *server_name )
{
	int sid1, rtn;
	struct hostent *sv_ip;	// $B%5!<%P(BIP$B%"%I%l%9(B
	struct sockaddr_in sv_addr;	/* $B%5!<%P%"%I%l%9(B */

	/* $B%3%M%/%7%g%s7?%=%1%C%H$N:n@.(B (socket) */
	sid1 = socket( AF_INET, SOCK_STREAM, 0 );
	if (sid1 < 0) {
		perror("sv:socket");
		close( sid1 );
		return -1;
	}
	/* $B%5!<%P$N#I#P%"%I%l%9$r<hF@(B */
	sv_ip = gethostbyname( server_name );
	if ( sv_ip == NULL ) {
		perror("sv:gethostbyname");
		close( sid1 );
		return -1;
	}
	/* $B%=%1%C%H$KL>A0$rIU2C(B (bind) */
	bzero( (char *)&sv_addr, sizeof(sv_addr) );
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons( port );
	memcpy( (char *)&sv_addr.sin_addr, (char *)sv_ip->h_addr, sv_ip->h_length );
	rtn = bind( sid1, (struct sockaddr *)&sv_addr, sizeof(sv_addr) );
	if ( rtn < 0 ) {
		perror("sv:bind");
		close( sid1 );
		return -1;
	}
	/* $B%/%i%$%"%s%H$+$i$N@\B3MW5a$N<uIU(B (listen) */
	rtn = listen( sid1, 5 );
	if ( rtn == -1 ) {
		perror("sv:listen");
		close( sid1 );
		return -1;
	}
	return sid1;
}

int InitSockClTCP( u_short port, char *server_name )
{
	int sid1, rtn;
	struct hostent *sv_ip;
	struct sockaddr_in sv_addr;
	unsigned int **addrptr;
	
	sid1 = socket( AF_INET, SOCK_STREAM, 0 );
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons( port );
	sv_ip = gethostbyname( server_name );
	addrptr = (unsigned int **)sv_ip->h_addr_list;
	sv_addr.sin_addr.s_addr = *(*addrptr);
	connect( sid1, (struct sockaddr *)&sv_addr, sizeof(sv_addr) );
	return sid1;
}

int InitSockSvUDP( u_short port )
{
	int sid1, rtn;
//	struct hostent *sv_ip;	// $B%5!<%P(BIP$B%"%I%l%9(B
	struct sockaddr_in sv_addr;	/* $B%5!<%P%"%I%l%9(B */

	/* $B%3%M%/%7%g%s7?%=%1%C%H$N:n@.(B (socket) */
	sid1 = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sid1 < 0) {
		perror("sv:socket");
		close( sid1 );
		return -1;
	}
	/* $B%=%1%C%H$KL>A0$rIU2C(B (bind) */
	bzero( (char *)&sv_addr, sizeof(sv_addr) );
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons( port );
	sv_addr.sin_addr.s_addr = INADDR_ANY;
	rtn = bind( sid1, (struct sockaddr *)&sv_addr, sizeof(sv_addr) );
	if ( rtn < 0 ) {
		perror("sv:bind");
		close( sid1 );
		return -1;
	}
	return sid1;
}

int InitSockClUDP( u_short port, char *server_name, struct sockaddr_in *sv_addr )
{
	int sid1;
	struct hostent *sv_ip;	// $B%5!<%P(BIP$B%"%I%l%9(B
	unsigned int **addrptr;

	/* $B%3%M%/%7%g%s7?%=%1%C%H$N:n@.(B (socket) */
	sid1 = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sid1 < 0) {
		perror("sv:socket");
		close( sid1 );
		return -1;
	}
	/* $B%=%1%C%H$KL>A0$rIU2C(B (bind) */
	bzero( (char *)sv_addr, sizeof*(sv_addr) );
	sv_addr->sin_family = AF_INET;
	sv_addr->sin_port = htons( port );
	/* $B%5!<%P$N#I#P%"%I%l%9$r<hF@(B */
	sv_ip = gethostbyname( server_name );
	if ( sv_ip == NULL ) {
		perror("sv:gethostbyname");
		close( sid1 );
		return -1;
	}
	addrptr = (unsigned int **)sv_ip->h_addr_list;
	sv_addr->sin_addr.s_addr = *(*addrptr);
	
	return sid1;
}

int AcceptSockSvTCP( int sid1 )
// accept
{
	int sid2;
	socklen_t cl_size;
	struct sockaddr_in cl_addr;	/* $B%/%i%$%"%s%H%"%I%l%9(B */

	cl_size = sizeof(cl_addr);
	sid2 = accept( sid1, (struct sockaddr *)&cl_addr, &cl_size );
	if ( sid2 < 0 ) {
		perror("sv:accept");
		close( sid1 );
		return -1;
	}
	return sid2;
}

void ShutdownSockSvTCP( int sid2 )
// shutdown, close
{
	int rtn;
	/* $B%=%1%C%H$N3+J|(B (shutdown,close) */
	rtn = shutdown(sid2, 2);
	if (rtn < 0) perror("sv:shutdown");
	close( sid2 );
}

void CloseSock( int sid1 )
// close
{
	close( sid1 );
}
#endif