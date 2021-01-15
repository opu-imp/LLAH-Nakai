#include "def_general.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "ws_cl.h"

SOCKET InitWinSockClTCP( u_short port, char *server_name )
// WinSockのクライアントを初期化
{
	SOCKET sock;
	WSADATA wsaData;
	struct sockaddr_in server;
	unsigned int **addrptr;
	
	// winsock2の初期化
	if ( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) {
		fprintf( stderr, "Error : WSAStartup failed\n" );
		return INVALID_SOCKET;
	}
	// ソケットの作成
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == INVALID_SOCKET ) {
		fprintf( stderr, "Error : socket error(%d)\n", WSAGetLastError() );
		return INVALID_SOCKET;
	}
	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons( port );
	server.sin_addr.S_un.S_addr = inet_addr( server_name );
	if ( server.sin_addr.S_un.S_addr == 0xffffffff ) {	// inet_addrが失敗
		struct hostent *host;
		host = gethostbyname( server_name );	// ホスト名からアドレスを取得
		if ( host == NULL ) {
			if ( WSAGetLastError() == WSAHOST_NOT_FOUND )
				fprintf( stderr, "Error : host %s not found\n", server_name );
			else	fprintf( stderr, "Error : gethostbyname\n" );
			return INVALID_SOCKET;
		}
		addrptr = (unsigned int **)host->h_addr_list;	// アドレスリストを取得
		while ( *addrptr != NULL ) {	// すべてのアドレスについて
			server.sin_addr.S_un.S_addr = *(*addrptr);
			if ( connect( sock, (struct sockaddr *)&server, sizeof(server) ) == 0 )	break;	// connect成功
			addrptr++;	// 次のアドレスで試す
		}
		if ( *addrptr == NULL ) {	// connectがすべて失敗
			fprintf( stderr, "Error : connect(%d)\n", WSAGetLastError() );
			return INVALID_SOCKET;
		}
	} else {	// inet_addr()が成功
		if ( connect( sock, (struct sockaddr *)&server, sizeof(server)) != 0 ) {
			fprintf( stderr, "Error : connect(%d)\n", WSAGetLastError() );
			return INVALID_SOCKET;
		}
	}
	return sock;
}

SOCKET InitWinSockSvTCP( u_short port, char *server_name )
{
	SOCKET sid1;
	WSADATA wsaData;
	struct hostent *sv_ip;
	struct sockaddr_in sv_addr;
	unsigned int **addrptr;
	
	WSAStartup(MAKEWORD(2,0), &wsaData);
	sid1 = socket( AF_INET, SOCK_STREAM, 0 );
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons( port );
//	sv_ip = gethostbyname( server_name );
//	addrptr = (unsigned int **)sv_ip->h_addr_list;
//	sv_addr.sin_addr.S_un.S_addr = *(*addrptr);
	sv_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind( sid1, (struct sockaddr *)&sv_addr, sizeof(sv_addr) );
	listen( sid1, 5 );
	
	return sid1;
}

SOCKET AcceptWinSockSvTCP( SOCKET sid1 )
{
	SOCKET sid2;
	int cl_size;
	struct sockaddr_in cl_addr;
	
	cl_size = sizeof( cl_addr );
	sid2 = accept( sid1, (struct sockaddr *)&cl_addr, &cl_size );
	
	return sid2;
}

SOCKET InitWinSockClUDP( u_short port, char *server_name, struct sockaddr_in *addr )
// WinSockのクライアントを初期化（UDP）
{
	SOCKET sock;
	WSADATA wsaData;
//	struct sockaddr_in addr;
	struct hostent *host;
	unsigned int **addrptr;

	WSAStartup(MAKEWORD(2,0), &wsaData);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	host = gethostbyname( server_name );	// ホスト名からアドレスを取得
	addrptr = (unsigned int **)host->h_addr_list;	// アドレスリストを取得
	addr->sin_addr.S_un.S_addr = *(*addrptr);
	
	return sock;
}

SOCKET InitWinSockSvUDP( u_short port )
{
	int rtn;
	SOCKET sock;
	WSADATA wsaData;
	struct sockaddr_in addr;
	struct hostent *host;
	unsigned int **addrptr;

	WSAStartup(MAKEWORD(2,0), &wsaData);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
//	addr.sin_addr.s_addr = INADDR_ANY;

//	host = gethostbyname( "kana" );	// ホスト名からアドレスを取得
//	addrptr = (unsigned int **)host->h_addr_list;	// アドレスリストを取得
//	addr.sin_addr.S_un.S_addr = *(*addrptr);


	rtn = bind( sock, (struct sockaddr *)&addr, sizeof(addr) );
//	printf("bind : %d\n", rtn);
	
	return sock;
}

void CloseWinSock( SOCKET sock )
// WinSockのクライアントを終了
{
	closesocket( sock );
	WSACleanup();
}
