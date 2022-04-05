// DlgConnect.cpp : implementation file
//

#include "stdafx.h"
#include "clt.h"
#include "DlgConnect.h"
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgConnect dialog


DlgConnect::DlgConnect(CWnd* pParent /*=NULL*/)
	: CDialog(DlgConnect::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgConnect)
	m_nick = _T("monkey");
	m_server = _T("localhost");
	m_port = _T("9000");
	//}}AFX_DATA_INIT
}


void DlgConnect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgConnect)
	DDX_Control(pDX, IDC_TCP_IP, m_btnTcp);
	DDX_Control(pDX, IDC_SPX_IPX, m_btnSpx);
	DDX_Text(pDX, IDC_NICKNAME, m_nick);
	DDV_MaxChars(pDX, m_nick, 30);
	DDX_Text(pDX, IDC_SERVER, m_server);
	DDV_MaxChars(pDX, m_server, 16);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDV_MaxChars(pDX, m_port, 16);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgConnect, CDialog)
	//{{AFX_MSG_MAP(DlgConnect)
	ON_BN_CLICKED(IDC_SPX_IPX, OnSpxIpx)
	ON_BN_CLICKED(IDC_TCP_IP, OnTcpIp)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgConnect message handlers

void DlgConnect::OnSpxIpx() 
{
	IsTcp = FALSE;
	m_btnSpx.SetCheck(1);
	m_btnTcp.SetCheck(0);
}

void DlgConnect::OnTcpIp() 
{
	IsTcp = TRUE;
	m_btnTcp.SetCheck(1);
	m_btnSpx.SetCheck(0);
}

void DlgConnect::OnOK() 
{
	BOOL err = FALSE;
	if ( UpdateData() ) 
	{
		if ( !err && m_nick == "" )
		{
			AfxMessageBox("Введите ваш ник.", MB_OK | MB_ICONSTOP );
			err = TRUE;
		}
		if ( !err && m_server == "" && IsTcp)
		{
			AfxMessageBox("Ошибка!\nВы не ввели адрес хоста. Вы должны ввести IP адрес удаленного хоста в десятичной нотации либо символьный эквивалент службы.", MB_OK | MB_ICONSTOP );
			err = TRUE;
		}
		if ( !err )
		{
			if ( IsTcp )
			{
				if ( inet_addr( m_server ) == INADDR_NONE )
				{	
					struct hostent *hp;
					hp = gethostbyname( m_server );
					if ( hp == NULL )
					{
						CString errmsg;
						errmsg.Format("Ошибка! Неизвестный хост: %s\n", m_server );
						AfxMessageBox(errmsg, MB_OK | MB_ICONSTOP );
						err = TRUE;
					}
				}
			}
			else
			{
				if ( !IsTcp && m_server == "" ) m_server = _T("255.255.255.255");

				if ( inet_addr( m_server ) == INADDR_NONE && strcmp(m_server, "255.255.255.255"))
				{
					CString errmsg;
					errmsg.Format("Ошибка! Неизвестный номер сети: %s\n", m_server );
					AfxMessageBox(errmsg, MB_OK | MB_ICONSTOP );
					err = TRUE;
				}
			}
			if ( !err )	CDialog::OnOK(); 
		}
	}
}

int DlgConnect::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	IsTcp = TRUE;
	return 0;
}

void DlgConnect::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (IsTcp)
	{
		m_btnTcp.SetCheck(1);
		m_btnSpx.SetCheck(0);
	}
	else
	{
		m_btnTcp.SetCheck(0);
		m_btnSpx.SetCheck(1);
	}
}
