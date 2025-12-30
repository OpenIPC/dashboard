#if !defined(AFX_DLGUSERMANAGE_H__7A41568E_AAC5_4EA2_A86F_FDD2B2CD8186__INCLUDED_)
#define AFX_DLGUSERMANAGE_H__7A41568E_AAC5_4EA2_A86F_FDD2B2CD8186__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgUserManage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgUserManage dialog

class CDlgUserManage : public CDialog
{
// Construction
public:
	CDlgUserManage(CWnd* pParent = NULL, LLONG hLoginID = 0, NET_DEVICE_TYPE emDevType = NET_PRODUCT_NONE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgUserManage)
	enum { IDD = IDD_DLG_USER };
	CString	m_csNewPasswd;
	CString	m_csCheckPasswd;
	CString	m_csOldPasswd;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUserManage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgUserManage)
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonSet();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG			m_hLoginID;
	NET_DEVICE_TYPE	m_emDevType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGUSERMANAGE_H__7A41568E_AAC5_4EA2_A86F_FDD2B2CD8186__INCLUDED_)
