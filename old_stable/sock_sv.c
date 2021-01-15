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
	struct hostent *sv_ip;	// サーバIPアドレス
	struct sockaddr_in sv_addr;	/* サーバアドレス */

	/* コネクション型ソケットの作成 (socket) */
	sid1 = socket( AF_INET, SOCK_STREAM, 0 );
	if (sid1 < 0) {
		perror("sv:socket");
		close( sid1 );
		return -1;
	}
	/* サーバのＩＰアドレスを取得 */
	sv_ip = gethostbyname( server_name );
	if ( sv_ip == NULL ) {
		perror("sv:gethostbyname");
		close( sid1 );
		return -1;
	}
	/* ソケットに名前を付加 (bind) */
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
	/* クライアントからの接続要求の受付 (listen) */
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
//	struct hostent *sv_ip;	// サーバIPアドレス
	struct sockaddr_in sv_addr;	/* サーバアドレス */

	/* コネクション型ソケットの作成 (socket) */
	sid1 = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sid1 < 0) {
		perror("sv:socket");
		close( sid1 );
		return -1;
	}
	/* ソケットに名前を付加 (bind) */
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
	struct hostent *sv_ip;	// サーバIPアドレス
	unsigned int **addrptr;

	/* コネクション型ソケットの作成 (socket) */
	sid1 = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sid1 < 0) {
		perror("sv:socket");
		close( sid1 );
		return -1;
	}
	/* ソケットに名前を付加 (bind) */
	bzero( (char *)sv_addr, sizeof*(sv_addr) );
	sv_addr->sin_family = AF_INET;
	sv_addr->sin_port = htons( port );
	/* サーバのＩＰアドレスを取得 */
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
	struct sockaddr_in cl_addr;	/* クライアントアドレス */

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
	/* ソケットの開放 (shutdown,close) */
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