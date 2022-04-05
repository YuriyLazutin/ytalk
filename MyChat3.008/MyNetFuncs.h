/***************************************************************************
                          MyNetFuncs.h  - description 
                             -------------------
    begin				: Mon 3 May 2004
    copyright			: (c) 2004 by Lazutin Yriy, all rights reserved.								
    email				: lazutin@newmail.ru
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/////	Header file for library MyNetFuncs.cpp
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYNETFUNCS_H__9AF7277C_E6E8_40A7_B1FB_E80628993549__INCLUDED_)
#define AFX_MYNETFUNCS_H__9AF7277C_E6E8_40A7_B1FB_E80628993549__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>
#include <wsipx.h>

extern char *program_name;

// Winsock library initialization
// return 0 if normal, error code if error
int WinSockInit(void);
 
// Print error (->stderr)(only for socket operations) 
void PrintError(char *, ... );


///////////////////////////////////////////////////////////////////////////////
///// MyTcpFuncs implementation

// set address for TCP/IP
void set_address( char *, char *, struct sockaddr_in *, char *);

// Create TCP server
// params:	first - pointer to the host name,
//			second - pointer to the port or service name
// return: socket in listening state if normal or INVALID_SOCKET if error
SOCKET CreateTcpServer( char *, char * );

// Create TCP client
// params:	first - pointer to the host name,
//			second - pointer to the port or service name
// return: connected socket if normal or INVALID_SOCKET if error
SOCKET CreateTcpClient( char *, char * );


/////////////////////////////////////////////////////////////////////////////
///// MySpxFuncs implementation

// set address for SPX/IPX
void set_novell_addr(char *, char *, struct sockaddr_ipx *, char *);

// Create SPX server
// params:	first - pointer to the host name,
//			second - pointer to the port or service name
// return: socket in listening state if normal or INVALID_SOCKET if error
SOCKET CreateSpxServer( char *, char * );

// Create SPX client
// params:	first - pointer to the net number,
//			second - pointer to the socket
// return: connected socket if normal or INVALID_SOCKET if error
SOCKET CreateSpxClient( char *, char *, char * );

#define DIAGN_MSG_CNT 3

// Search Spx Server
// if normal return 0 and server address  or -1 if error
int FindSpxServer( char *, char *, struct sockaddr_ipx *);

#endif // !defined(AFX_MYNETFUNCS_H__9AF7277C_E6E8_40A7_B1FB_E80628993549__INCLUDED_)
