#if !defined(AFX_DLGCFGALARMPOWERFAULT_H__ABC3909B_6805_479A_AD30_4B34F7164986__INCLUDED_)
#define AFX_DLGCFGALARMPOWERFAULT_H__ABC3909B_6805_479A_AD30_4B34F7164986__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmPowerFault.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmPowerFault dialog

class CDlgCfgAlarmPowerFault : public CDialog
{
// Construction
public:
	BOOL getInfo();
	BOOL showInfo();
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	CDlgCfgAlarmPowerFault(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmPowerFault)
	enum { IDD = IDD_DLG_CFG_ALARM_POWERFAULT };
	CComboBox	m_cmbPower;
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmPowerFault)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmPowerFault)
	virtual BOOL OnInitDialog();
	afx_msg void OnPowerfaultButtonGet();
	afx_msg void OnPowerfaultButtonSet();
	afx_msg void OnPowerfaultButtonEventhandler();
	afx_msg void OnSelchangePowerfaultComboPower();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LLONG				m_lLoginId;
	CFG_POWERFAULT_INFO*	m_pstuPowerFault;
	int					m_nPowerIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMPOWERFAULT_H__ABC3909B_6805_479A_AD30_4B34F7164986__INCLUDED_)
