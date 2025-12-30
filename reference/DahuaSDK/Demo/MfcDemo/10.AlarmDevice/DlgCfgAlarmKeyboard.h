#if !defined(AFX_DLGCFGALARMKEYBOARD_H__666AD4CB_B8E8_46EA_8BEF_D9B062B5760E__INCLUDED_)
#define AFX_DLGCFGALARMKEYBOARD_H__666AD4CB_B8E8_46EA_8BEF_D9B062B5760E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmKeyboard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmKeyboard dialog

class CDlgCfgAlarmKeyboard : public CDialog
{
// Construction
public:
	CDlgCfgAlarmKeyboard(CWnd* pParent = NULL, LLONG lLoginId = NULL, int nMaxKeyboard = 10);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmKeyboard)
	enum { IDD = IDD_DLG_CFG_ALARMKEYBOARD };
	CButton	m_chkEnable;
	CComboBox	m_cbAddress;
	CComboBox	m_cbBaudRate;
	CComboBox	m_cbStopBit;
	CComboBox	m_cbVerify;
	CComboBox	m_cbDataBit;
	CComboBox	m_cbSerialPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmKeyboard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmKeyboard)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeKeyboardCmbSerialport();
	afx_msg void OnSelchangeKeyboardCmbAddress();
	afx_msg void OnKeyboardBtnGet();
	afx_msg void OnKeyboardBtnSet();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg();
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	void DlgToStu();
	void StuToDlg();
	void CleanDlg();

private:
	LLONG					m_lLoginId;
	CFG_ALARMKEYBOARD_INFO*	m_pstuInfo;
	int						m_nMaxKeyboard;
	int						m_nCurKeyboardCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMKEYBOARD_H__666AD4CB_B8E8_46EA_8BEF_D9B062B5760E__INCLUDED_)
