/***************************************************************************
                          svr.cpp  - description 
                             -------------------
    begin				: Mon 3 May 2004
    copyright			: (c) 2004 by Lazutin Yriy, all rights reserved.								
    email				: lazutin@newmail.ru
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
/////	Defines the entry point for the console application.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "svr.h"
#include <winsock2.h>
#include <afxmt.h>
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

#define MAX_ERRORS 3

CWinThread *diagn_srvs;

//////////////////////
///// connections list

#define MAX_NICKNAME_LENGTH	32
struct connection
{
	struct connection *next;
	CWinThread *m_thread;
	SOCKET s;
	char user_nickname[MAX_NICKNAME_LENGTH];
	BOOL zombie;
};

CCriticalSection conn_is_busy;
int nConnNumb;
struct connection *connections, *lastconnection;

///// end connections list
//////////////////////////

// Print usage information
void PrintHelp(FILE *stream);

// MFC initialization ( used for work with threads )
// return 0 if normal, -1 if error
int MFCInit(void);

// serve new connection
// if normal return 0 else return error code
UINT serve( LPVOID /* struct connection* */);

// Диагностическая служба для определения spx сервера
UINT diagnostic_service( LPVOID );

///// remove connection from list
///// if normal return 0 else return -1 
int KillConnection( struct connection *);

///// Send message to everyone connected clients
void SendToEveryone(struct packet *);

///// Add nickname into the list of connected users
///// first parameter - pointer to the structure what included added nick
///// (must be fild following fields: buf, head.reclen) 
///// second parameter - pointer to the structure what define connection 
void AddNick (struct packet *, struct connection *);

///// send to user nicks of all connected chaters
void UpdateNickList (struct connection *);

///// Remove nick for all connected users exept client whose nick will removed
void RemoveNick (const struct connection *);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	program_name = argv[0];
	
	struct sockaddr peer;
	int peerlen;

	char *hname, *sname, *proto;
	SOCKET s, s1;

	BOOL IsTcp = TRUE;
	
	////////////////
	///// get option

	// set default parameters
	sname = "9000";
	hname = NULL;
	proto = "tcp";

	for ( int i= 1; i<argc; i++)
	{
		int optsz = strlen(argv[i]);
		if (argv[i][0] == '-' && optsz == 2)
		{
			switch (argv[i][1])
			{
				case 'h':
					i++;
					if (i<argc)	hname = argv[i];
					else
					{
						PrintHelp(stderr);
						fprintf(stderr, "Error! Absence argument with option -h.\n");
						return -1;
					}
					break;

				case 's':
					i++;
					if (i<argc)	sname = argv[i];
					else
					{
						PrintHelp(stderr);
						fprintf(stderr, "Error! Absence argument with option -s.\n");
						return -1;
					}
					break;

				case 'p':
					i++;
					if (i<argc)	proto = argv[i];
					else
					{
						PrintHelp(stderr);
						fprintf(stderr, "Error! Absence argument with option -p.\n");
						return -1;
					}
					break;

				case '?':
					PrintHelp(stdout);
					return 0;

				default:
					PrintHelp(stderr);
					fprintf(stderr, "Error! Unknown option %s\n", argv[i] );
					return -1;
			}
		}
		else
		{
			PrintHelp(stderr);
			fprintf(stderr, "Error! Not correct option %s\n", argv[i] );
			return -1;
		}
	}
	
	///// end get option
	////////////////////

	nRetCode = MFCInit();
	if ( nRetCode != 0 ) return nRetCode;
	else printf("MFC initialization - OK.\n");

	nRetCode = WinSockInit();
	if ( nRetCode != 0 ) return nRetCode;
	else printf("Winsock library initialization - OK.\n");
	
	if ( strcmp(proto, "tcp" ) == 0 || strcmp(proto, "TCP" ) == 0 ) 		 
		s = CreateTcpServer(hname, sname);
	else if ( strcmp(proto, "spx") == 0 || strcmp(proto, "SPX") == 0)
	{
		IsTcp = FALSE;
		s = CreateSpxServer(hname, sname);
		diagn_srvs = AfxBeginThread(diagnostic_service, NULL, THREAD_PRIORITY_BELOW_NORMAL);	
	}
	else 
	{
		fprintf(stderr, "Error! Unknown protocol: %s\n", proto);
		fprintf(stderr, "Must be tcp or spx.\n");
		return -1;
	}
	
	if ( s == INVALID_SOCKET  )
	{
		fprintf(stderr, "error creating server.\n");
		return INVALID_SOCKET;
	}
	else printf("Server was created successfully.\n");

	nConnNumb = 0;
	
	do
	{
		peerlen = sizeof( peer );

		///// permits an incoming connection attempt on a socket
		s1 = accept( s, &peer, &peerlen );
		if ( s == INVALID_SOCKET )
		{
			PrintError("error calling accept.\n");
			return INVALID_SOCKET;
		}

		printf("Accept new connection - OK.\n");
		printf("New socket descriptor - %u\n", (unsigned int)s1); 

		if (IsTcp)
		{
			///// set alive option
			int keepon = 1;
			nRetCode = setsockopt(s1, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepon, sizeof(keepon));
			if ( nRetCode == SOCKET_ERROR )
			{
				PrintError("error calling setsockopt with option SO_KEEPALIVE.\n");
				return SOCKET_ERROR;
			}
			else printf("Set option SO_KEEPALIVE - OK.\n");
		}

		struct connection *tmp;
		tmp = (struct connection *)malloc( sizeof( struct connection ) );
		if ( tmp == NULL )
		{
			fprintf(stderr, "Not enough memory for accept new connection.\n");
			fprintf(stderr, "Socket descriptor - %u\n",(unsigned int)s1 );
			fprintf(stderr, "Connection wasn't created.\n");
			nRetCode = closesocket(s1);
			if (nRetCode == SOCKET_ERROR)
				PrintError("Fatal error! Can't close socket %u.\nServer must be stopped!\n",(unsigned int)s1);
		}
		else
		{
			///// add new connection into the list
			tmp->next = NULL;
			tmp->s = s1;
			tmp->zombie = FALSE;
			tmp->m_thread = AfxBeginThread(serve, (LPVOID)tmp, THREAD_PRIORITY_BELOW_NORMAL);
			
			CSingleLock csl(&conn_is_busy);
			csl.Lock();
			if (nConnNumb == 0 )
				connections = tmp;
			else
				lastconnection->next=tmp;
			lastconnection=tmp;
			nConnNumb++;
			csl.Unlock();
		}
	} while(1);

	WSACleanup( );
	return nRetCode;
}

void PrintHelp(FILE *stream)
{
	fprintf(stream, "\n/***************************************************************\n");
	fprintf(stream, "\t\tMyChat server. Version 0.1\n");
	fprintf(stream, "\t\t-------------------\n");
	fprintf(stream, "begin\t\t: Mon 3 May 2004\n");
	fprintf(stream, "Copyright\t: (c) 2004 by Lazutin Yriy, all rights reserved.\n");
	fprintf(stream, "email\t\t: lazutin@newmail.ru\n");
	fprintf(stream, "***************************************************************/\n\n");
	
	fprintf(stream, "Usage: svr.exe [-h host] [-s port] [-p protocol] [-?]\n");
	fprintf(stream, "-h host - ip address or host name\n");
	fprintf(stream, "-s port - port or service name\n");
	fprintf(stream, "-p protocol - protocol (tcp or spx)\n");
	fprintf(stream, "-? - print this help and exit\n\n");
}

// MFC initialization ( used for work with threads )
// return 0 if normal, -1 if error
int MFCInit(void)
{
	// initialize MFC and print error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return -1;
	}
	return 0;
}



UINT serve( LPVOID pParam)
{
	int nRetCode;
	struct connection *con;
	con = (struct connection *)pParam;
	struct packet pak;

	BOOL run = TRUE;
	int nErrCnt = 0;
	while ( run )
	{
		nRetCode = readmsg( con->s, &pak, sizeof( pak.buf ) );
		if ( nRetCode < 0 )
		{
			int nerr = WSAGetLastError();
			if (nerr == WSAECONNRESET)
			{
					printf("Client %u was reset connection.\nConnection will killed.\n",(unsigned int)con->s);
					con->zombie = TRUE;
					RemoveNick( con );
					nRetCode = KillConnection(con);
					if ( nRetCode != 0 )
					{
						fprintf(stderr, "Fatal error! Can't kill connection.\nServer must be stoped.\n");
						return 2;
					}
					return 1;
			}
			else
			{
				nErrCnt++;
				fprintf(stderr, "Client (socket) %u:\n", (unsigned int)con->s);
				fprintf(stderr, "readmsg returned error code.\n");
				if (nErrCnt> MAX_ERRORS)
				{
					fprintf(stderr, "Too much errors. Connection will killed.\n");
					con->zombie = TRUE;
					RemoveNick( con );
					nRetCode = KillConnection(con);
					if ( nRetCode != 0 )
					{
						fprintf(stderr, "Fatal error! Can't kill connection.\nServer must be stoped.\n");
						return 2;
					}
					return 1;
				}
				Sleep(3000);
			}	
		}
		else if (nRetCode == 0)
		{
			printf("Client %u was disconnect.\n", (unsigned int)con->s );
			RemoveNick( con );
			run = FALSE;
		}
		else
		{
			///// Data was received correctly
			nErrCnt = 0;

			switch ( ntohs(pak.head.msgtype) )
			{
				case MSG_TEXT:
					SendToEveryone(&pak);
					break;

				case MSG_JOIN_ME:
					{
						BOOL err = FALSE;
						unsigned short n = ntohs(pak.head.reclen);
						///// Analyze size of nickname
						if (n<=30)
						{
							///// Add nickname into the connections list
							memcpy(con->user_nickname, pak.buf, n);
							con->user_nickname[n] = '\0';
						}
						else // size of nickname more then permissible
						{
							err = TRUE;
							fprintf(stderr, "Client %u: ", (unsigned int)con->s);
							fprintf(stderr, "Can't add user nickname.\n");
							fprintf(stderr,"Nickname length to long.\n");
						}
						if ( !err )
						{
							struct connection *tmp;
							tmp = connections;


							///// set critical section as busy
							CSingleLock csl(&conn_is_busy);
							csl.Lock();

							///// send nickname of new user all connected clients
							while (tmp != NULL )
							{
								if (tmp != con)	AddNick( &pak, tmp);
								tmp = tmp->next;
							}

							///// send list of all connected users
							UpdateNickList( con );
							
							///// set critical section as free
							csl.Unlock();
						}
					}
					break;

				case MSG_EXIT:
					RemoveNick( con );
					nRetCode = KillConnection(con);
					if ( nRetCode != 0 )
					{
						fprintf(stderr, "Fatal error! Can't kill connection.\nServer must be stoped.\n");
						return 2;
					}
					return 0;

			//	case MSG_CMD:
					///// пока команды не поддерживаются 
			//		break;

				default:
					///// получили неизвесный тип данных
					///// вывести сообщение об этом и отбросить данные
					fprintf(stderr, "Received unknown type of message from user %u.\n",( unsigned int )con->s);
					break;
			}
		}
	}
	nRetCode = KillConnection(con);
	if ( nRetCode != 0 )
	{
		fprintf(stderr, "Fatal error! Can't kill connection.\nServer must be stoped.\n");
		return 2;
	}
	return 0;
}

// Диагностическая служба для определения spx сервера
UINT diagnostic_service( LPVOID pParam )
{
	// Объявляем и заполняем структуру для адреса удаленного 
	// диагностического сервиса
	struct sockaddr_ipx diagn_svr;
	int diagn_svr_len = sizeof(diagn_svr);

	memset(&diagn_svr, 0, sizeof(diagn_svr));

	diagn_svr.sa_family = AF_IPX;
	diagn_svr.sa_socket = htons(0x4567);
	
	// создаем сокет для посылки широковещательных сообщений

	SOCKET sp = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if ( sp == INVALID_SOCKET  )
	{
		PrintError("Creating diagnostic socket: error calling socket.\n");
		exit (1);
	}
	else printf("Create diagnostic IPX socket - OK.\n");

	int rc;
	
	rc = bind(sp, (struct sockaddr *)&diagn_svr, sizeof(diagn_svr));
	if ( rc == SOCKET_ERROR)
	{
		PrintError("error calling bind.\n");
		exit(1);
	}
	else printf("Associate a local address with a diagnostic socket - OK.\n"); 

	// прием широковещательных пакетов и отправка ответов
	char buf[100];
	
	
	while (1)
	{
		// read received message
		rc = recvfrom(sp, buf, sizeof(buf), 0, (struct sockaddr *)&diagn_svr, &diagn_svr_len);
		if ( rc == SOCKET_ERROR )
		{
			PrintError("error calling recvfrom with diagnostic.\n");
			exit(1);
		}
				
		// send diagnostic message
		strcpy(buf, "re: diagnostic");
		rc = sendto(sp, buf, strlen(buf), MSG_DONTROUTE, (struct sockaddr*)&diagn_svr, sizeof(diagn_svr));
		if (rc == SOCKET_ERROR )
		{
			PrintError("error calling sendto with diagnostic.\n");
			exit (1);
		}	
	} 

	return 0;
}


///// remove connection from list
///// dec connection counter
///// close SOCKET
///// free allocated memory
///// if normal return 0 else return -1 
int KillConnection( struct connection * con)
{
	///// Set critical section as busy
	CSingleLock csl(&conn_is_busy);
	csl.Lock();
	
	///// Delete connection from list
	if (con == connections) // if deleted first item in list
		connections = con->next;
	else
	{	
		// search item before deleted connection
		struct connection *tmp;
		tmp = connections;
		while (tmp->next != con && tmp->next != NULL) tmp = tmp->next;
		// we can't find deleted item
		if (tmp->next != con)
		{
			fprintf(stderr, "Can't find connection from list.\n");
			return -1;
		}
		// connect before and next items
		tmp->next = con->next;
	}
	nConnNumb--;
	csl.Unlock();

	int rc;
	rc = closesocket(con->s);
	if (rc == SOCKET_ERROR)
	{
		PrintError("Can't close socket %u\n",(unsigned int)con->s);
		return -1;
	}
	free(con);
	return 0;
}

///// Send received message to everyone connected clients
void SendToEveryone(struct packet *pak)
{	
		int n = ntohs(pak->head.reclen);
		struct connection *tmp;
		tmp = connections;

		///// Set critical section as busy 
		CSingleLock csl(&conn_is_busy);
		csl.Lock();
		
		while (tmp != NULL )
		{
			if (!tmp->zombie)
			{
				BOOL run = TRUE;
				int nErrCnt = 0;
				while (run)
				{
					int rc = send( tmp->s, (char *)pak, n + sizeof(pak->head), 0);
					if ( rc == SOCKET_ERROR  )
					{
						nErrCnt++;
						rc = WSAGetLastError();
						if ( rc == WSAENOBUFS )	Sleep(100);
						else
						{
							PrintError("Error calling send.\n");
							tmp->zombie = TRUE;
							run = FALSE;
						}
						if (nErrCnt > MAX_ERRORS) run = FALSE;
					} 
					else run = FALSE;
				}
			}
			tmp = tmp->next;
		}
		csl.Unlock();
}

///// Remove nickname from list connected users
void RemoveNick (const struct connection *con)
{
	struct packet rnick;
	rnick.head.msgtype = htons( MSG_RM_NICK );
	unsigned short n = strlen( con->user_nickname );
	rnick.head.reclen = htons( n );
	strcpy(rnick.buf, con->user_nickname);

	struct connection *tmp;
	tmp = connections;

	///// Set critical section as busy 
	CSingleLock csl(&conn_is_busy);
	csl.Lock();
	
	while (tmp != NULL )
	{
		if ( tmp != con && !tmp->zombie)
		{
			BOOL run = TRUE;
			int nErrCnt = 0;
			while (run)
			{
				int rc = send( tmp->s, (char *)&rnick, n + sizeof(rnick.head), 0);
				if ( rc == SOCKET_ERROR  )
				{
					nErrCnt++;
					rc = WSAGetLastError();
					if ( rc == WSAENOBUFS )	Sleep(100);
					else
					{
						PrintError("Error calling send.\n");
						tmp->zombie = TRUE;
						run = FALSE;
					}
					if (nErrCnt > MAX_ERRORS) run = FALSE;
				} 
				else run = FALSE;
			}
		}
		tmp = tmp->next;
	}
	csl.Unlock();
}


///// Add nickname to list connected users
///// anick - pointer to the structure what included added nick
///// (must be fill following fields: buf, head.reclen) 
///// con - pointer to the structure what define connection 
void AddNick (struct packet *anick, struct connection *con)
{

	anick->head.msgtype = htons( MSG_ADD_NICK );
	unsigned short n = ntohs(anick->head.reclen);

	if (!con->zombie)
	{
		BOOL run = TRUE;
		int nErrCnt = 0;
		while (run)
		{
			int rc = send( con->s, (char *)anick, n + sizeof(anick->head), 0);
			if ( rc == SOCKET_ERROR  )
			{
				nErrCnt++;
				rc = WSAGetLastError();
				if ( rc == WSAENOBUFS )	Sleep(100);
				else
				{
					PrintError("Error calling send.\n");
					con->zombie = TRUE;
					run = FALSE;
				}
				if (nErrCnt > MAX_ERRORS) run = FALSE;
			} 
			else run = FALSE;
		}
	}
}

///// send to user nicks of all connected chaters
void UpdateNickList (struct connection *con)
{
	struct packet nicks;
	nicks.head.msgtype = htons( MSG_UPD_NICKS );
	unsigned short n;
	char *tmp;
	tmp = nicks.buf;
	struct connection *tmpcon;
	tmpcon = connections;
	int nicklen;
	while ( tmpcon != NULL )
	{
		
		nicklen = strlen( tmpcon->user_nickname );
		if (tmp - nicks.buf + nicklen +1 < sizeof(nicks.buf) )
		{
			strcpy( tmp, tmpcon->user_nickname);
			tmp+=nicklen;
			*tmp = '\0';
			tmp++;
		}
		else
		{
			n = tmp - nicks.buf;
			nicks.head.reclen = htons(n);

			
			if (!con->zombie)
			{
				BOOL run = TRUE;
				int nErrCnt = 0;
				while (run)
				{
					int rc = send( con->s, (char *)&nicks, n + sizeof(nicks.head), 0);
					if ( rc == SOCKET_ERROR  )
					{
						nErrCnt++;
						rc = WSAGetLastError();
						if ( rc == WSAENOBUFS )	Sleep(100);
						else
						{
							PrintError("Error calling send.\n");
							con->zombie = TRUE;
							run = FALSE;
						}
						if (nErrCnt > MAX_ERRORS) run = FALSE;
					} 
					else run = FALSE;
				}
			}

			tmp = nicks.buf;
		}
		tmpcon = tmpcon->next;
	}

	n = tmp - nicks.buf;
	if ( n != 0 )
	{
		nicks.head.reclen = htons(n);

		if (!con->zombie)
		{
			BOOL run = TRUE;
			int nErrCnt = 0;
			while (run)
			{
				int rc = send( con->s, (char *)&nicks, n + sizeof(nicks.head), 0);
				if ( rc == SOCKET_ERROR  )
				{
					nErrCnt++;
					rc = WSAGetLastError();
					if ( rc == WSAENOBUFS )	Sleep(100);
					else
					{
						PrintError("Error calling send.\n");
						con->zombie = TRUE;
						run = FALSE;
					}
					if (nErrCnt > MAX_ERRORS) run = FALSE;
				} 
				else run = FALSE;
			}
		}		
	}
}

