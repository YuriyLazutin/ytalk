#if !defined(AFX_DLGCONNECT_H__AD325D33_CC68_458B_8811_FE599D639383__INCLUDED_)
#define AFX_DLGCONNECT_H__AD325D33_CC68_458B_8811_FE599D639383__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgConnect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgConnect dialog

class DlgConnect : public CDialog
{
// Construction
public:
	BOOL IsTcp;
	DlgConnect(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgConnect)
	enum { IDD = IDD_CONNECTION_INFO };
	CButton	m_btnTcp;
	CButton	m_btnSpx;
	CString	m_nick;
	CString	m_server;
	CString	m_port;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgConnect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgConnect)
	afx_msg void OnSpxIpx();
	afx_msg void OnTcpIp();
	virtual void OnOK();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONNECT_H__AD325D33_CC68_458B_8811_FE599D639383__INCLUDED_)
