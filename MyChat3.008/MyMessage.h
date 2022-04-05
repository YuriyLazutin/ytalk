/***************************************************************************
                          MyMessage.h  - description 
                             -------------------
    begin				: Mon 3 May 2004
    copyright			: (c) 2004 by Lazutin Yriy, all rights reserved.								
    email				: lazutin@newmail.ru
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/////	Defines my message and functions for work it.
/////////////////////////////////////////////////////////////////////////////

#if !defined(MY_MESSAGE_H_3_05_2004)
#define MY_MESSAGE_H_3_05_2004

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>


///// define types of message

#define MSG_TEXT		0
#define MSG_JOIN_ME		1
#define MSG_EXIT		2
#define MSG_CMD			3
#define MSG_RM_NICK		4
#define MSG_ADD_NICK	5
#define MSG_UPD_NICKS	6
			
///// end define types of message

struct msghead
{
	unsigned short reclen;
	unsigned short msgtype;
	unsigned long msgflags; // reserved
};

struct packet
{
	struct msghead head;
	char buf[1024];
};

int readn( SOCKET s, char *buf, size_t len);


///// read message
int readmsg( SOCKET s, struct packet *buf, size_t len);


#endif // !defined(MY_MESSAGE_H_3_05_2004)