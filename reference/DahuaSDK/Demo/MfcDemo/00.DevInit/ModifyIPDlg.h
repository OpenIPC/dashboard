#if !defined(AFX_HANGEIP_H__7E5C1911_7F38_4C3D_9AD4_19B5E4B2550B__INCLUDED_)
#define AFX_HANGEIP_H__7E5C1911_7F38_4C3D_9AD4_19B5E4B2550B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// hangeIP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ChangeIP dialog

class ModifyIPDlg : public CDialog
{
// Construction
public:
	ModifyIPDlg(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(ChangeIP)
	enum { IDD = IDD_MODIFYIP_DIALOG };
	int		m_IpVersion;
	CString	m_IP;
	CString	m_strPassWord;
	CString	m_strUserName;
	CString	m_strSubnetMask;
	CString	m_strGateWay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChangeIP)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ChangeIP)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HANGEIP_H__7E5C1911_7F38_4C3D_9AD4_19B5E4B2550B__INCLUDED_)
