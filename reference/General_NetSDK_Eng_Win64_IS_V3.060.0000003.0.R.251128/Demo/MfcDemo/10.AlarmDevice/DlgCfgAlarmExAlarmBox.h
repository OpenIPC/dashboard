#if !defined(AFX_DLGCFGALARMEXALARMBOX_H__E4C29DE6_2728_4DDE_9BA6_97B704383A16__INCLUDED_)
#define AFX_DLGCFGALARMEXALARMBOX_H__E4C29DE6_2728_4DDE_9BA6_97B704383A16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmExAlarmBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmExAlarmBox dialog

class CDlgCfgAlarmExAlarmBox : public CDialog
{
// Construction
public:
	CDlgCfgAlarmExAlarmBox(CWnd* pParent = NULL, LLONG lLoginId = NULL, int nMaxExAlarmBox = MAX_EXALARMBOX_NUM);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmExAlarmBox)
	enum { IDD = IDD_DLG_CFG_ALARMEXALARMBOX };
		// NOTE: the ClassWizard will add data members here
	CButton		m_chkEnable;
	CComboBox	m_cbBaudRate;
	CComboBox	m_cbStopBit;
	CComboBox	m_cbVerify;
	CComboBox	m_cbDataBit;
	CComboBox	m_cbSerialPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmExAlarmBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmExAlarmBox)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeExAlarmBoxCmbSerialport();
	afx_msg void OnExAlarmBoxBtnGet();
	afx_msg void OnExAlarmBoxBtnSet();
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
	CFG_EXALARMBOX_INFO*	m_pstuInfo;
	int						m_nMaxExAlarmBox;
	int						m_nCurExAlarmBoxCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMEXALARMBOX_H__E4C29DE6_2728_4DDE_9BA6_97B704383A16__INCLUDED_)
