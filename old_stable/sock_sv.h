int InitSockSvTCP( u_short port, char *server_name );
int AcceptSockSvTCP( int sid1 );
void ShutdownSockSvTCP( int sid2 );
void CloseSock( int sid1 );
int InitSockSvUDP( u_short port );
int InitSockClUDP( u_short port, char *server_name, struct sockaddr_in *sv_addr );
int InitSockClTCP( u_short port, char *server_name );
