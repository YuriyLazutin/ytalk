/***************************************************************************
                          MyNetFuncs.cpp  - description 
                             -------------------
    begin				: Mon 3 May 2004
    copyright			: (c) 2004 by Lazutin Yriy, all rights reserved.								
    email				: lazutin@newmail.ru
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/////	Functions for work with TCP/IP and SPX/IPX.
/////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "MyNetFuncs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Winsock library initialization
// return 0 if normal, error code if error
int WinSockInit(void)
{
	// !!! Не забудь подключить Ws2_32.lib
	WORD wVersionRequested;
	WSADATA wsaData;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	int err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.*/
		PrintError("could not find a usable WinSock DLL\n");
		return err;
	}
	return 0;
}

// Print error (->stderr)(only for socket operations) 
void PrintError(char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	fprintf(stderr, "%s:\n", program_name);
	vfprintf(stderr, fmt, ap );
	va_end( ap );
	int err = WSAGetLastError();
	fprintf(stderr, "%s\n", strerror(err) );
	fprintf(stderr, "Error code : %d\n", err );
}

/////////////////////////////////////////////////////////////////////////////
///// MyTcpFuncs

// set address for TCP/IP
static void set_address(char *hname, char *sname, struct sockaddr_in *sap, char *protocol)
{
	memset( sap, 0, sizeof( *sap ) );
	sap->sin_family = AF_INET;

	if ( hname != NULL )
	{
		unsigned long nIP = inet_addr( hname );
		if ( nIP == INADDR_NONE && strcmp(hname, "255.255.255.255"))
		{		
			struct hostent *hp;
			hp = gethostbyname( hname );
			if ( hp == NULL )
			{
				fprintf(stderr, "Unknown host: %s\n", hname );
				exit(1);
			}
			sap->sin_addr = *( struct in_addr *)hp->h_addr;
		}
		else sap->sin_addr.s_addr = nIP;
	}
	else
		sap->sin_addr.s_addr = htonl( INADDR_ANY );

	char *endptr;
	unsigned short port = (unsigned short) strtol( sname, &endptr, 0 );
	if ( *endptr == '\0' )
		sap->sin_port = htons( port );
	else
	{
		struct servent *sp;
		sp = getservbyname( sname, protocol );
		if ( sp == NULL )
		{
			fprintf(stderr, "Unknown service: %s\n", sname);
			exit(1);
		}
		sap->sin_port = sp->s_port;
	}
}

// Create TCP server
// return: socket in listening state if normal or INVALID_SOCKET if error
SOCKET CreateTcpServer( char *hname, char *sname )
{
	struct sockaddr_in local;
	SOCKET s;
	const int on = 1;

	set_address( hname, sname, &local, "tcp");

	///// create a socket that is bound to a specific service provider
	s = socket( AF_INET, SOCK_STREAM, 0);
	if ( s == INVALID_SOCKET  )
	{
		PrintError("error calling socket.\n");
		return INVALID_SOCKET;
	}
	else printf("Create TCP socket - OK.\n");

	///// set a socket options
	int rc = setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on) );	
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling setsockopt with option SO_REUSEADDR.\n");
		return INVALID_SOCKET;
	}
	else printf("Set option SO_REUSEADDR - OK.\n");

	///// associate a local address with a socket
	rc = bind( s, (struct sockaddr *)&local, sizeof(local) );
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling bind.\n");
		return INVALID_SOCKET;
	}
	else printf("Associate a local address with a TCP socket - OK.\n");
	
	
	///// place a socket in a state in which it is listening for
	///// an incoming connection
	rc = listen( s, SOMAXCONN );
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling listen.\n");
		return INVALID_SOCKET;
	}
	else printf("Place socket in listening state  - OK.\n");
	return s;
}

// Create TCP client
// params:	first - pointer to the host name,
//			second - pointer to the port or service name
// return: connected socket if normal or INVALID_SOCKET if error
SOCKET CreateTcpClient( char *hname, char *sname )
{
	struct sockaddr_in peer;
	SOCKET s;

	set_address( hname, sname, &peer, "tcp");

	///// create a socket that is bound to a specific service provider
	s = socket( AF_INET, SOCK_STREAM, 0);
	if ( s == INVALID_SOCKET  )
	{
		PrintError("error calling socket.\n");
		return INVALID_SOCKET;
	}
	else printf("Create TCP socket - OK.\n");
	
	///// The connect function establishes a connection to a specified socket.
	int rc = connect(s, (struct sockaddr*)&peer, sizeof(peer) );
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling connect.\n");
		return INVALID_SOCKET;
	}
	else printf("Place socket in connecting state  - OK.\n");

	return s;
}

/////////////////////////////////////////////////////////////////////////////
///// MySpxFuncs

static void set_novell_addr(char *hname, char *sname, struct sockaddr_ipx *sap, char *protocol)
{
	memset( sap, 0, sizeof( *sap ) );
	sap->sa_family = AF_IPX; 

	// set netnum
	if ( hname != NULL )
	{
		unsigned long nIP = inet_addr( hname );
		if ( nIP == INADDR_NONE && strcmp(hname, "255.255.255.255"))
		{		
			fprintf(stderr, "Unknown netnum: %s\n", hname );
			exit(1);
		}
		memcpy( sap->sa_netnum, &nIP, sizeof(sap->sa_netnum) );
	}
	else
		memset( sap->sa_netnum, 0, sizeof(sap->sa_netnum) );

	// set node address
	memset( sap->sa_nodenum, 0xff, sizeof( sap->sa_nodenum ) );
	
	// set socket
	char *endptr;
	unsigned short sock = (unsigned short) strtol( sname, &endptr, 0 );
	if ( *endptr == '\0' )
		sap->sa_socket = htons( sock );
	else
	{
		struct servent *sp;
		sp = getservbyname( sname, protocol );
		if ( sp == NULL )
		{
			fprintf(stderr, "Unknown service: %s\n", sname);
			exit(1);
		}
		sap->sa_socket = sp->s_port;
	}
}

// Create SPX server
// return: socket in listening state if normal or INVALID_SOCKET if error 
SOCKET CreateSpxServer( char *hname, char *sname )
{
	struct sockaddr_ipx local;
	SOCKET s;
	const int on = 1;
	
	set_novell_addr( hname, sname, &local, "spx");

	///// create a socket that is bound to a specific service provider
	s = socket( AF_IPX, SOCK_STREAM, NSPROTO_SPX);
	if ( s == INVALID_SOCKET  )
	{
		PrintError("error calling socket.\n");
		return INVALID_SOCKET;
	}
	else printf("Create SPX socket - OK.\n");

	///// set a socket options
	int rc = setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on) );	
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling setsockopt with option SO_REUSEADDR.\n");
		return INVALID_SOCKET;
	}
	else printf("Set option SO_REUSEADDR - OK.\n");

	///// associate a local address with a socket
	rc = bind( s, (struct sockaddr *)&local, sizeof(local) );
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling bind.\n");
		return INVALID_SOCKET;
	}
	else printf("Associate a local address with a SPX socket - OK.\n");

	int sz = sizeof(local);
	rc = getsockname(s, (struct sockaddr *)&local, &sz);
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling getsockname.\n");
		return INVALID_SOCKET;
	}

	// print netnum
	printf("Our SPX netnum:");
	unsigned long tmpIP;
	memcpy(&tmpIP, local.sa_netnum, sizeof(unsigned long) );
	tmpIP = ntohl( tmpIP );
	unsigned char NetAddr[4];
	memcpy(&NetAddr, &tmpIP, sizeof(NetAddr) );
	for ( int i=0; i<4; i++) printf(" %02.2X", NetAddr[i]);
	printf("\n"); 

	// print nodenum
	printf("Our SPX nodenum in net order:");
	for ( i=0; i<6; i++) printf(" %02.2X",(unsigned char)local.sa_nodenum[i]);
	printf("\n");
	
	
	///// place a socket in a state in which it is listening for
	///// an incoming connection
	rc = listen( s, SOMAXCONN );
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling listen.\n");
		return INVALID_SOCKET;
	}
	else printf("Place socket in listening state  - OK.\n");
	return s;
}

// Create SPX client
// params:	netnum - pointer to the net number,
//			node - pointer to the node
//			sock - pointer to the socket
// return: connected socket if normal or INVALID_SOCKET if error
SOCKET CreateSpxClient( char *netnum, char *node, char *sock )
{
	struct sockaddr_ipx peer;

	int rc;

	set_novell_addr( netnum, sock, &peer, "spx");
	
	if (node == NULL)
	{
		rc = FindSpxServer( peer.sa_netnum, (char *)&peer.sa_socket, &peer );

		/////// decomment this
		//memset(&peer.sa_netnum[3], 0x54, 1);
		//memset(&peer.sa_netnum[2], 0xf1, 1);
		//memset(&peer.sa_netnum[1], 0x3d, 1);
		//memset(&peer.sa_netnum[0], 0xe1, 1);
		//memset(peer.sa_nodenum, 0, 6);
		//memset(&peer.sa_nodenum[5], 1, 1);
		////// end decomment this
		
		///// comment this
		if ( rc == -1 )
		{
			fprintf(stderr, "Cann't find spx server.\n");
			return INVALID_SOCKET;
		}
		///// end comment this
	}	
	else
		memcpy( peer.sa_nodenum, node, 6 );

	SOCKET s;

	int on = 1;

	
	///// create a socket that is bound to a specific service provider
	s = socket( AF_IPX, SOCK_STREAM, NSPROTO_SPX);
	if ( s == INVALID_SOCKET )
	{
		PrintError("error calling socket.\n");
		return INVALID_SOCKET;
	}
	else printf("Create SPX socket - OK.\n");

	///// The connect function establishes a connection to a specified socket.
	rc = connect(s, (struct sockaddr*)&peer, sizeof(peer) );
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling connect.\n");
		return INVALID_SOCKET;
	}
	else printf("Place socket in connected state  - OK.\n");

	
	// print netnum
	printf("Remoute SPX netnum:");
	unsigned long tmpIP;
	memcpy(&tmpIP, peer.sa_netnum, 4 );
	tmpIP = ntohl( tmpIP );
	unsigned char NetAddr[4];
	memcpy(&NetAddr, &tmpIP, sizeof(NetAddr) );
	for ( int i=0; i<4; i++) printf(" %02.2X", NetAddr[i]);
	printf("\n"); 

	// print nodenum
	printf("Remoute SPX nodenum in net order:");
	for ( i=0; i<6; i++) printf(" %02.2X",(unsigned char)peer.sa_nodenum[i]);
	printf("\n");

	return s;
}

// Search Spx Server
// if normal return 0 and server address  or -1 if error
int FindSpxServer( char *netnum, char *sock, struct sockaddr_ipx *sap)
{
	// Объявляем и заполняем структуру для адреса удаленного 
	// диагностического сервиса
	struct sockaddr_ipx diagn;
	int diagn_len = sizeof(diagn);

	memset(&diagn, 0, sizeof(diagn));

	char diagn_sock[16];
	strcpy( diagn_sock, "17767");
	
	set_novell_addr( NULL, diagn_sock, &diagn, "ipx");

	memcpy( diagn.sa_netnum, netnum, 4);

	// Заполняем номер сети, семейство и сокет, т.к. они уже известны
	sap->sa_family = AF_IPX;
	memcpy( sap->sa_netnum, netnum, 4);
	memcpy( &sap->sa_socket, sock, 2);
	
	// создаем сокет для посылки широковещательных сообщений

	SOCKET s = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if ( s == INVALID_SOCKET  )
	{
		PrintError("Creating diagnostic socket: error calling socket.\n");
		return -1;
	}
	else printf("Create diagnostic IPX socket - OK.\n");

	///// set a socket options
	int on = 1;
	int rc = setsockopt( s, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(on) );	
	if ( rc == SOCKET_ERROR )
	{
		PrintError("error calling setsockopt with option SO_BROADCAST.\n");
		return -1;
	}
	else printf("Set option SO_BROADCAST - OK.\n");

	
	// отправка широковещательных пакетов и прием ответов
	int i = DIAGN_MSG_CNT;
	char buf[100];
	
	fd_set allfd;
	fd_set readmask;
	FD_ZERO( &allfd);
	FD_ZERO(&readmask);
	FD_SET(s, &allfd);
	struct timeval tv;
	
	// структура для адресов принимаемых пакетов
	struct sockaddr tmp;
	int tmp_len = sizeof(tmp);

	while (i>0)
	{
		// send diagnostic message
		strcpy(buf, "diagnostic");
		rc = sendto(s, buf, strlen(buf), /*MSG_DONTROUTE*/0, (struct sockaddr*)&diagn, sizeof(diagn));
		if (rc == SOCKET_ERROR )
		{
			PrintError("error calling sendto.\n");
			return -1;
		}

		BOOL run = TRUE;
		while (run)
		{
			// read received messages
			tv.tv_sec = 3;
			tv.tv_usec = 0;
	
			readmask = allfd;
			rc = select(s+1, &readmask, NULL, NULL, &tv);
			if ( rc == SOCKET_ERROR )
			{
				PrintError("Error calling select.\n");
				return -1;
			}
			else if ( rc == 0 ) // time left
			{
				run = FALSE;
				i--;
			}
			else
			{
				if ( FD_ISSET(s, &readmask) )
				{
					rc = recvfrom(s, buf, sizeof(buf), 0, &tmp, &tmp_len);
					if ( rc == SOCKET_ERROR )
					{
						PrintError("error calling recvfrom.\n");
						return -1;
					}
					else
					{
						if (rc == 0 )					
						{
							printf("Received end of file. Diagnostic server was disconnect.\n");
						}
						else
						{
							run = FALSE;
							i = -1;
						}
					}				
				}
				else
				{
					fprintf(stderr, "select returned invalid socket.\n");
					return -1;
				}
			}
		}		
	}

	if ( i == -1 ) // server was found
	{
		struct sockaddr_ipx * remoute;
		remoute = (struct sockaddr_ipx *)&tmp;
		memcpy( sap->sa_nodenum, remoute->sa_nodenum, 6);
		return 0;
	}
	fprintf(stderr, "Can't find remoute server.\n");
	return -1;
}