#if !defined(AFX_DLGCFGALARMURGENCY_H__132E2A03_6025_40E6_8311_B1435C26170E__INCLUDED_)
#define AFX_DLGCFGALARMURGENCY_H__132E2A03_6025_40E6_8311_B1435C26170E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmUrgency.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmUrgency dialog

class CDlgCfgAlarmUrgency : public CDialog
{
// Construction
public:
	CDlgCfgAlarmUrgency(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor
	
public:
	BOOL getInfo();
	BOOL showInfo();
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmUrgency)
	enum { IDD = IDD_DLG_CFG_ALARM_URGENCY };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmUrgency)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmUrgency)
	virtual BOOL OnInitDialog();
	afx_msg void OnUrgencyButtonGet();
	afx_msg void OnUrgencyButtonSet();
	afx_msg void OnUrgencyButtonEventhandler();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG				m_lLoginId;
	CFG_URGENCY_INFO	m_stuUrgency;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMURGENCY_H__132E2A03_6025_40E6_8311_B1435C26170E__INCLUDED_)
