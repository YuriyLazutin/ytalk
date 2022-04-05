// c.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "winsock2.h"
#include "wsipx.h"

int WinSockInit(void)
{
	// !!! Не забудь подключить Ws2_32.lib
	WORD wVersionRequested;
	WSADATA wsaData;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	int err = WSAStartup( wVersionRequested, &wsaData );
	return 0;
}

int main(int argc, char* argv[])
{
	WinSockInit();
	
	struct sockaddr_ipx local;
	int len = sizeof(local);

	memset(&local, 0, sizeof(local));

	local.sa_family = AF_IPX;
	local.sa_socket = htons(9000);
	memset( local.sa_netnum, 0, 4);
	memset(local.sa_nodenum, 0xff, 6);
	
	//memset(&local.sa_netnum[3], 0x54, 1);
	//memset(&local.sa_netnum[2], 0xf1, 1);
	//memset(&local.sa_netnum[1], 0x3d, 1);
	//memset(&local.sa_netnum[0], 0xe1, 1);
	//memset(&local.sa_nodenum[5], 1, 1);
	
	SOCKET s;
	s = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if ( s == INVALID_SOCKET  )
	{
		printf("Creating diagnostic socket: error calling socket.\n");
		return 0;
	}
	
	int rc;

	///// set a socket options
	int on = 1;
	rc = setsockopt( s, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(on) );	
	if ( rc == SOCKET_ERROR )
	{
		printf("error calling setsockopt with option SO_BROADCAST.\n");
		return 0;
	} 
		
	char buf[100];
	
	int i=0;
	while (i<10)
	{
		strcpy(buf, "diagnostic");
		rc = sendto(s, buf, strlen(buf),/*MSG_DONTROUTE*/0, (struct sockaddr*)&local, sizeof(local));
		if (rc == SOCKET_ERROR )
		{
			printf("error calling sendto. error code %d\n", WSAGetLastError());
			return 0;
		}
		
		rc = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&local, &len);
		if ( rc == SOCKET_ERROR )
		{
			printf("error calling recvfrom. error code %d\n", WSAGetLastError());
			return 0;
		}
		buf[rc] = '\0';	printf(buf); printf("\n");

		i++;
		
	} 
	return 0;
}

