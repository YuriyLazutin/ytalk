#if !defined(AFX_DLGMYCHAT_H__DC9485B0_1F0E_4B00_B16C_EDD8D6CBE36D__INCLUDED_)
#define AFX_DLGMYCHAT_H__DC9485B0_1F0E_4B00_B16C_EDD8D6CBE36D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMyChat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgMyChat dialog
#include <winsock2.h>


class DlgMyChat : public CDialog
{
// Construction
public:
	SOCKET s;
	DlgMyChat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgMyChat)
	enum { IDD = IDD_CHAT };
	CListBox	m_lstNicks;
	CListBox	m_lstMain;
	CString	m_my_msg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgMyChat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgMyChat)
	virtual void OnOK();
	afx_msg void OnFileExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMYCHAT_H__DC9485B0_1F0E_4B00_B16C_EDD8D6CBE36D__INCLUDED_)
