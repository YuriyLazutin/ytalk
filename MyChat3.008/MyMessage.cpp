#include "stdafx.h"
#include "MyMessage.h"

int readn( SOCKET s, char *buf, size_t len)
{
	int cnt, nRetCode;

	cnt = len;
	while( cnt > 0 )
	{
		nRetCode = recv( s, buf, cnt, 0 );
		if ( nRetCode == SOCKET_ERROR )
		{
			if (  WSAGetLastError() == WSAEINTR )
				continue;
			return -1;
		}
		if ( nRetCode == 0 )   // End of file
			return len - cnt;
		buf += nRetCode;
		cnt -= nRetCode;
	}
	return len;
}


///// read message
int readmsg( SOCKET s, struct packet *buf, size_t len)
{
	unsigned short reclen;
	int nRetCode;

	///// Чтение заголовка фиксированной длинны
	nRetCode = readn( s, (char *)&buf->head, sizeof( struct msghead ) );
	if ( nRetCode != sizeof( struct msghead ) )
		return nRetCode < 0 ? -1 : 0;
	reclen = ntohs( buf->head.reclen );
	//printf("reclen = %d\n",reclen);

	if ( reclen > len )
	{
		// Не хватает места в буфере для размещения данных -
		// отбросить их и вернуть код ошибки
		while ( reclen > 0 )
		{
			nRetCode = readn( s, buf->buf, len );
			if ( nRetCode != len )
				return nRetCode < 0 ? -1 : 0;
			reclen -= len;
			if ( reclen < len )
				len = reclen;
		}
		SetLastError( WSAEMSGSIZE );
		return -1;
	}

	// Читаем сообщение
	nRetCode = readn( s, buf->buf, reclen );
	if ( nRetCode != reclen )
		return nRetCode < 0 ? -1 : 0;
	return nRetCode;
}
