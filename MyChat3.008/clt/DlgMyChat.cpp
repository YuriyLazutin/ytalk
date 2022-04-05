// DlgMyChat.cpp : implementation file
//

#include "stdafx.h"
#include "clt.h"
#include "DlgMyChat.h"
#include <winsock2.h>
#include "MyMessage.h"
//#include "MyNetFuncs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgMyChat dialog


DlgMyChat::DlgMyChat(CWnd* pParent /*=NULL*/)
	: CDialog(DlgMyChat::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgMyChat)
	m_my_msg = _T("");
	//}}AFX_DATA_INIT
}


void DlgMyChat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgMyChat)
	DDX_Control(pDX, IDC_NICKS, m_lstNicks);
	DDX_Control(pDX, IDC_CHAT_MSG, m_lstMain);
	DDX_Text(pDX, IDC_MY_MSG, m_my_msg);
	DDV_MaxChars(pDX, m_my_msg, 1024);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgMyChat, CDialog)
	//{{AFX_MSG_MAP(DlgMyChat)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgMyChat message handlers

void DlgMyChat::OnOK() 
{
	UpdateData();

	if (m_my_msg != "")
	{
		///// send data	
		struct packet pak;

		strcpy( pak.buf, m_my_msg );
		unsigned short n = strlen( pak.buf );
		pak.head.reclen = htons( n );
		pak.head.msgtype = htons( MSG_TEXT );
		if ( send( s, (char *)&pak, n + sizeof( pak.head ), 0 ) == SOCKET_ERROR  )
		{
			AfxMessageBox("Error calling send\n", MB_OK | MB_ICONSTOP );
			return ;
		}
		m_my_msg = "";
		UpdateData(FALSE);
		///// end send data
	}
}


void DlgMyChat::OnFileExit() 
{
	struct packet pak;
	pak.head.reclen = htons( 0 );
	pak.head.msgtype = htons( MSG_EXIT );
	if ( send( s, (char *)&pak, sizeof(pak.head), 0 ) == SOCKET_ERROR  )
		AfxMessageBox("Error calling send.\n", MB_OK | MB_ICONSTOP );
	CDialog::OnOK();
	//EndModalLoop(IDOK);
}

