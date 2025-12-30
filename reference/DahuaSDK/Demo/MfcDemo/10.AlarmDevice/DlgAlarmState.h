#if !defined(AFX_DLGALARMSTATE_H__B771A4A8_0A4D_40C2_98A1_FB59E41A39EB__INCLUDED_)
#define AFX_DLGALARMSTATE_H__B771A4A8_0A4D_40C2_98A1_FB59E41A39EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAlarmState.h : header file
//

typedef struct  
{
	NET_ALARM_CHANNEL_TYPE emType;
	char* pszType;
}Demo_AlarmState_Type;

const Demo_AlarmState_Type DemoAlarmType[] = 
{
	{NET_ALARM_CHANNEL_TYPE_ALL, "All"},
	{NET_ALARM_CHANNEL_TYPE_ALARMIN, "AlarmIn"},
	{NET_ALARM_CHANNEL_TYPE_ALARMOUT, "AlarmOut"},
	{NET_ALARM_CHANNEL_TYPE_ALARMBELL, "AlarmBell"},
 	{NET_ALARM_CHANNEL_TYPE_EXALARMIN, "ExAlarmIn"},
 	{NET_ALARM_CHANNEL_TYPE_EXALARMOUT, "ExAlarmOut"},
}; 

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmState dialog

class CDlgAlarmState : public CDialog
{
// Construction
public:
	CDlgAlarmState(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor
	~CDlgAlarmState();
	
	bool getInfo();
	void showInfo(int nCurSel);
	BOOL getCfgFromDevice();
	void deleteMemory();

// Dialog Data
	//{{AFX_DATA(CDlgAlarmState)
	enum { IDD = IDD_DLG_ALARM_STATE };
	CEdit	m_edExAlarmOutRetCount;
	CEdit	m_edExAlarmOutCount;
	CEdit	m_edExAlarmInRetCount;
	CEdit	m_edExAlarmInCount;
	CComboBox	m_cmbExAlarmOutChannel;
	CComboBox	m_cmbExAlarmInChannel;
	CButton	m_ckExAlarmOutState;
	CButton	m_ckExAlarmInState;
	CComboBox	m_cmbAlarmType;
	CEdit	m_edAlarmOutRetCount;
	CEdit	m_edAlarmOutCount;
	CEdit	m_edAlarmInRetCount;
	CEdit	m_edAlarmInCount;
	CEdit	m_edAlarmBellRetCount;
	CEdit	m_edAlarmBellCount;
	CComboBox	m_cmbAlarmOutChannel;
	CComboBox	m_cmbAlarmInChannel;
	CComboBox	m_cmbAlarmBellChannel;
	CButton	m_ckAlarmOutState;
	CButton	m_ckAlarmInState;
	CButton	m_ckAlarmBellState;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarmState)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarmState)
	afx_msg void OnAlarmstateButton();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeAlarmstateComboAlarmbellChannel();
	afx_msg void OnSelchangeAlarmstateComboAlarminChannel();
	afx_msg void OnSelchangeAlarmstateComboAlarmoutChannel();
	afx_msg void OnSelchangeAlarmstateComboAlarmType();
	afx_msg void OnSelchangeAlarmstateComboExalarmoutChannel();
	afx_msg void OnSelchangeAlarmstateComboExalarminChannel();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG m_lLoginId;
	NET_CLIENT_ALARM_CHANNELS_STATE m_stuAlarmChannelState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARMSTATE_H__B771A4A8_0A4D_40C2_98A1_FB59E41A39EB__INCLUDED_)
