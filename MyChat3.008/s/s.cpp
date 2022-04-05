// s.cpp : Defines the entry point for the console application.
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
	
	SOCKET s;
	s = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if ( s == INVALID_SOCKET  )
	{
		printf("Creating diagnostic socket: error calling socket.\n");
		return 0;
	}
	
	int rc;
	
	rc = bind(s, (struct sockaddr *)&local, sizeof(local));
	if ( rc == SOCKET_ERROR)
	{
		printf("error calling bind. error code %d\n", WSAGetLastError());
		return 0;
	} 
	
	char buf[100];

	while (1)
	{
		rc = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&local, &len);
		if ( rc == SOCKET_ERROR )
		{
			printf("error calling recvfrom. error code %d\n", WSAGetLastError());
			return 0;
		}
		buf[rc] = '\0';	printf(buf); printf("\n");
				
		
		strcpy(buf, "re: diagnostic");
		rc = sendto(s, buf, strlen(buf), MSG_DONTROUTE,	(struct sockaddr*)&local, sizeof(local));
		if (rc == SOCKET_ERROR )
		{
			printf("error calling sendto.\n");
			return 0;
		}	
	} 
	return 0;
}

