#if !defined(AFX_DLGCFGBATTERYLOWPOWER_H__21F19466_D3C8_48E6_93BD_4F1B7FD1A376__INCLUDED_)
#define AFX_DLGCFGBATTERYLOWPOWER_H__21F19466_D3C8_48E6_93BD_4F1B7FD1A376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgBatteryLowPower.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgBatteryLowPower dialog

class CDlgCfgBatteryLowPower : public CDialog
{
// Construction
public:
	CDlgCfgBatteryLowPower(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgBatteryLowPower)
	enum { IDD = IDD_DLG_CFG_BATTERYLOWPOWER };
	CButton	m_chkEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgBatteryLowPower)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgBatteryLowPower)
	virtual BOOL OnInitDialog();
	afx_msg void OnBatterylowpowerBtnHandler();
	afx_msg void OnBatterylowpowerBtnGet();
	afx_msg void OnBatterylowpowerBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL getInfo();
	BOOL showInfo();
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	
private:
	LLONG				m_lLoginId;
	CFG_BATTERY_LOW_POWER_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGBATTERYLOWPOWER_H__21F19466_D3C8_48E6_93BD_4F1B7FD1A376__INCLUDED_)
