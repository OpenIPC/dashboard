#if !defined(AFX_INITDEVICEDLG_H__D10EE1B5_CD3C_444D_A244_595B3F3007E5__INCLUDED_)
#define AFX_INITDEVICEDLG_H__D10EE1B5_CD3C_444D_A244_595B3F3007E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InitDeviceDlg.h : header file
//

//#include "dhnetsdk.h"

/////////////////////////////////////////////////////////////////////////////
// CInitDeviceDlg dialog

class CInitDeviceDlg : public CDialog
{
// Construction
public:
	CInitDeviceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInitDeviceDlg)
	enum { IDD = IDD_INITDEVICE_DIALOG };

	CString	m_strPwd;
	CString	m_strRig;
	CString	m_strUserName;
	CString	m_strPwdRestWay;
    CString m_strConfirmPwd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInitDeviceDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInitDeviceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	char m_szMac[DH_MACADDR_LEN];
	int m_nIndex;
    int m_byPwdResetWay;
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
  
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INITDEVICEDLG_H__D10EE1B5_CD3C_444D_A244_595B3F3007E5__INCLUDED_)
