#if !defined(AFX_GETIP_H__6003F0CE_CA46_4DDE_9132_5F6E293320FF__INCLUDED_)
#define AFX_GETIP_H__6003F0CE_CA46_4DDE_9132_5F6E293320FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GetIP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// GetIP dialog

class GetIPDlg : public CDialog
{
// Construction
public:
	GetIPDlg(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(GetIP)
	enum { IDD = IDD_GETIP_DIALOG };
	CIPAddressCtrl	m_ctlEndIP;
	CIPAddressCtrl	m_ctlStartIP;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GetIP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GetIP)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETIP_H__6003F0CE_CA46_4DDE_9132_5F6E293320FF__INCLUDED_)
