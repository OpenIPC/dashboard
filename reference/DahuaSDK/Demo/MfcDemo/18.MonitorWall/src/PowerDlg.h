#if !defined(AFX_POWERDLG_H__55BC27C1_1C24_40E8_87D0_D9F580B2F7F9__INCLUDED_)
#define AFX_POWERDLG_H__55BC27C1_1C24_40E8_87D0_D9F580B2F7F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PowerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPowerDlg dialog

class CPowerDlg : public CDialog
{
// Construction
public:
	CPowerDlg(LLONG lLoginID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPowerDlg)
	enum { IDD = IDD_DIALOG_POWER };
	CComboBox	m_cbPower;
	CComboBox	m_cbOutput;
	CComboBox	m_cbWall;
	CComboBox	m_cbBlock;
	//}}AFX_DATA

	LLONG		m_lLoginID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPowerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitUI();
	AV_CFG_MonitorWallBlock* GetSpliceScreen(LPCTSTR lpszMonitorName, int nBlockID);

	std::vector<AV_CFG_MonitorWall> m_vecWall;
//	std::vector<AV_CFG_MonitorWallBlock> m_vecSplice;

	// Generated message map functions
	//{{AFX_MSG(CPowerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeComboMonitorwall();
	afx_msg void OnSelchangeComboBlock();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POWERDLG_H__55BC27C1_1C24_40E8_87D0_D9F580B2F7F9__INCLUDED_)
