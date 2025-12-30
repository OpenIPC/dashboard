#if !defined(AFX_DLGCFGRCEMERGENCYCALL_H__D78984A4_FBB4_4190_962F_58834C283667__INCLUDED_)
#define AFX_DLGCFGRCEMERGENCYCALL_H__D78984A4_FBB4_4190_962F_58834C283667__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgRCEmergencyCall.h : header file
//

typedef struct tagRCE_AlarmType
{
    EM_CFG_RCEMERGENCY_CALL_TYPE    emAlarmType;
    const char*                     szInfo;
}RCE_AlarmType;

const RCE_AlarmType stuDemoRCEAlarmType[] = 
{
    {EM_CFG_RCEMERGENCY_CALL_UNKNOWN, "Unknown"},
    {EM_CFG_RCEMERGENCY_CALL_FIRE, "Fire"},
    {EM_CFG_RCEMERGENCY_CALL_DURESS, "Duress"},
    {EM_CFG_RCEMERGENCY_CALL_ROBBER, "Robber"},
    {EM_CFG_RCEMERGENCY_CALL_MEDICAL, "Medical"},
};

typedef struct tagRCE_Mode
{
    EM_CFG_RCEMERGENCY_MODE_TYPE    emMode;
    const char*                     szInfo;
}RCE_MODE;

const RCE_MODE stuDemoRCEMode[] = 
{
    {EM_CFG_RCEMERGENCY_MODE_UNKNOWN, "Unknown"},
    {EM_CFG_RCEMERGENCY_MODE_KEYBOARD, "Keyboard"},
    {EM_CFG_RCEMERGENCY_MODE_WIRELESS_CONTROL, "WirelessControl"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgRCEmergencyCall dialog

class CDlgCfgRCEmergencyCall : public CDialog
{
// Construction
public:
	CDlgCfgRCEmergencyCall(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgRCEmergencyCall)
	enum { IDD = IDD_DLG_CFG_RCEMERGENCYCALL };
	CComboBox	m_cmbChn;
	CComboBox	m_cmbMode;
	CComboBox	m_cmbAlarmType;
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgRCEmergencyCall)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgRCEmergencyCall)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgRCEmergencyCallBtnEventhandler();
	afx_msg void OnCfgRCEmergencyCallBtnGet();
	afx_msg void OnCfgRCEmergencyCallBtnSet();
	afx_msg void OnSelchangeCfgRCEmergencyCallCmbChn();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void InitDlg();
    BOOL getInfo();
    BOOL showInfo();
    BOOL SetConfigToDevice();
    BOOL GetConfigFromDevice();
    
private:
    LLONG                       m_lLoginId;
	CFG_RCEMERGENCY_CALL_INFO   m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGRCEMERGENCYCALL_H__D78984A4_FBB4_4190_962F_58834C283667__INCLUDED_)
