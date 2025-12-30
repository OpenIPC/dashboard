#if !defined(AFX_DLGCFGNETABORT_H__074FD9CB_3CE8_42EE_A98E_16E431DB4885__INCLUDED_)
#define AFX_DLGCFGNETABORT_H__074FD9CB_3CE8_42EE_A98E_16E431DB4885__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgNetAbort.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetAbort dialog

class CDlgCfgNetAbort : public CDialog
{
// Construction
public:
	CDlgCfgNetAbort(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgNetAbort)
	enum { IDD = IDD_DLG_CFG_NETABORT };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgNetAbort)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgNetAbort)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgnetabortBtnEventhandler();
	afx_msg void OnCfgnetabortBtnGet();
	afx_msg void OnCfgnetabortBtnSet();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    BOOL getInfo();
    BOOL showInfo();
    BOOL SetConfigToDevice();
    BOOL GetConfigFromDevice();

private:
    LLONG               m_lLoginId;
	CFG_NETABORT_INFO   m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGNETABORT_H__074FD9CB_3CE8_42EE_A98E_16E431DB4885__INCLUDED_)
