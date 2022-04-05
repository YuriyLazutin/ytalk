/***************************************************************************
                          clt.cpp  - description 
                             -------------------
    begin				: Mon 3 May 2004
    copyright			: (c) 2004 by Lazutin Yriy, all rights reserved.								
    email				: lazutin@newmail.ru
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/////	Defines the entry point for the console application.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "clt.h"
#include <winsock2.h>
#include "DlgMyChat.h"
#include "DlgConnect.h"
#include "MyMessage.h"
#include "MyNetFuncs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

char *program_name;

int MFCInit(void);

// Read incoming messages
UINT ReadIncMsg(LPVOID pParam);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	
	int nRetCode = 0;
	program_name = argv[0];
	
	nRetCode = MFCInit();
	if ( nRetCode != 0 ) return nRetCode;
	else printf("MFC initialization - OK.\n");

	nRetCode = WinSockInit();
	if ( nRetCode != 0 ) return nRetCode;
	else printf("Winsock library initialization - OK.\n");

	DlgMyChat dmc;
	dmc.Create(IDD_CHAT);
	dmc.ShowWindow(SW_SHOW);

	DlgConnect dc;
	nRetCode = dc.DoModal();
	if (nRetCode == IDOK )
	{
		SOCKET s;
		
		if ( dc.IsTcp )
		{	
			// Создание клиента tcp
			char host[16];
			char port[16];
			strcpy(host, dc.m_server);
			strcpy(port, dc.m_port);
			s = CreateTcpClient( host, port);
			if (s == INVALID_SOCKET)
			{
				fprintf(stderr, "Cann't connect to server.\n");
				return -1;
			}
		}
		else
		{
			// Создание клиента spx
			char netnum[16];
			char spx_sock[16];
			
			strcpy(spx_sock, dc.m_port);
			if ( strcmp(dc.m_server, "") != 0 )
			{
				strcpy(netnum, dc.m_server);
				s = CreateSpxClient( netnum, NULL, spx_sock);
			}
			else
				s = CreateSpxClient( NULL, NULL, spx_sock );
			if (s == INVALID_SOCKET)
			{
				fprintf(stderr, "Cann't connect to server.\n");
				return -1;
			}
		}
		
		dmc.s=s;
		CWinThread *readThrd;
		readThrd = AfxBeginThread( ReadIncMsg, (LPVOID)&dmc );
		struct packet pak;
		pak.head.msgtype = htons( MSG_JOIN_ME );
		strcpy(pak.buf, dc.m_nick );
		unsigned short n = strlen(pak.buf);
		pak.head.reclen = htons(n);
		if ( send( s, (char *)&pak, n + sizeof( pak.head ), 0 ) < 0 )
		{
			fprintf(stderr, "Error calling send.\n" );
			return 1;
		}
		
		dmc.RunModalLoop();

		readThrd->ExitInstance();
		closesocket(s);
	}

	WSACleanup( );

	return nRetCode;
}


// MFC initialization
// return 0 if normal, -1 if error
int MFCInit(void)
{
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return -1;
	}
	return 0;
}


UINT ReadIncMsg(LPVOID pParam)
{
	int nRetCode;
	DlgMyChat *dmc;
	dmc = (DlgMyChat *)pParam;
	struct packet pak;

	while (1)
	{
		nRetCode = readmsg( dmc->s, &pak, sizeof( pak.buf ) );
		if ( nRetCode < 0 )
		{
			fprintf(stderr, "Readmsg returned error code.\n");
			AfxMessageBox("Server was disconnect.\n", MB_OK | MB_ICONSTOP );
			exit(1);
		}
		else if (nRetCode == 0)
		{
			AfxMessageBox("Server was disconnect.", MB_OK | MB_ICONSTOP );
			exit(1);
		}
		else
		{
			///// Data was received correctly
			switch ( ntohs(pak.head.msgtype) )
			{
				case MSG_TEXT:
					pak.buf[ntohs(pak.head.reclen)]='\0';
					dmc->m_lstMain.AddString(pak.buf);
					break;

				case MSG_ADD_NICK:
					pak.buf[ntohs(pak.head.reclen)]='\0';
					dmc->m_lstNicks.AddString(pak.buf);
					break;

				case MSG_RM_NICK:
					pak.buf[ntohs(pak.head.reclen)]='\0';
					dmc->m_lstNicks.DeleteString(dmc->m_lstNicks.FindString(0, pak.buf));
					break;

				case MSG_UPD_NICKS:
					{
						unsigned short n, l;
						char buf[32], *t;
						t = pak.buf;
						n = ntohs( pak.head.reclen );
						while ( n > 0 )
						{
							strcpy( buf, t );
							dmc->m_lstNicks.AddString(buf);
							l = strlen(buf)+1;
							t+= l;
							n-= l;
						}
					}
					break;

				default:
					AfxMessageBox("Received unknown message type.", MB_OK | MB_ICONSTOP );
			}
		}
	} 
	return 0;
} 