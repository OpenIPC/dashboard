#if !defined(AFX_DLGCFGNETWORK_H__8D21CD5D_F73C_4C8F_9927_6C802C003ADE__INCLUDED_)
#define AFX_DLGCFGNETWORK_H__8D21CD5D_F73C_4C8F_9927_6C802C003ADE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgNetwork.h : header file
//
#define WM_ALARM_INFO	(WM_USER+199)

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetwork dialog

class CDlgCfgNetwork : public CDialog
{
// Construction
public:
	CDlgCfgNetwork(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor
	void GetNetworkPara();

// Dialog Data
	//{{AFX_DATA(CDlgCfgNetwork)
	enum { IDD = IDD_DLG_CFG_NETWORK };
	CIPAddressCtrl	m_ctlMask;
	CIPAddressCtrl	m_ctlIp;
	CIPAddressCtrl	m_ctlGateway;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgNetwork)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgNetwork)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnCancel();
	afx_msg void OnBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG				m_lLoginID;
	CFG_NETWORK_INFO	m_stuNetwork;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGNETWORK_H__8D21CD5D_F73C_4C8F_9927_6C802C003ADE__INCLUDED_)
