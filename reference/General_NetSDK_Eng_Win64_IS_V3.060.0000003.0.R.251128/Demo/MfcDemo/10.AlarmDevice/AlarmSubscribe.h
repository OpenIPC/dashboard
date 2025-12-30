#if !defined(AFX_ALARMSUBSCRIBE_H__C7BFC1C5_D321_4CEE_BE90_FB6A67BD954B__INCLUDED_)
#define AFX_ALARMSUBSCRIBE_H__C7BFC1C5_D321_4CEE_BE90_FB6A67BD954B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlarmSubscribe.h : header file
//
#include <afxmt.h>

typedef struct 
{
	NET_ALARM_TYPE	emAlarmType;
	char*			pszName;
}DemoConfirmEvent;

const DemoConfirmEvent stuDemoConfirmEvent[] = 
{
	{NET_ALARM_LOCAL, "AlarmLocal"},
	{NET_ALARM_ALARMEXTENDED, "AlarmExtended"},
	{NET_ALARM_TEMP, "Temperature"},
	{NET_ALARM_URGENCY, "Urgency"},
    {NET_ALARM_RCEMERGENCYCALL, "RCEmergencyCall"},
};

#define WM_ALARM_INFO	(WM_USER+199)

#define MAX_MSG_SHOW	(10000)

/////////////////////////////////////////////////////////////////////////////
// AlarmSubscribe dialog

class AlarmSubscribe : public CDialog
{
// Construction
public:
	AlarmSubscribe(CWnd* pParent = NULL, LLONG hLoginID = NULL, UINT32 uiAlarmIn = 0);   // standard constructor
// Dialog Data
	//{{AFX_DATA(AlarmSubscribe)
	enum { IDD = IDD_DLG_SUBSCRIBE };
	CComboBox	m_cbAlarmType;
	CComboBox	m_cbChannel;
	CListCtrl m_ctrAlarmRptList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AlarmSubscribe)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AlarmSubscribe)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSub();
	afx_msg void OnButtonStop();
	afx_msg void OnBtnConfirm();
	afx_msg void OnSelchangeSubscribeCmbType();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnAlarmInfo(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	InitListControl();
	void	GetTimeStringByStruct(NET_TIME stuTime, char *pTime);

	void	UpdateEventList();

	void	InsertAlarmExEvent(BYTE* byBuf, int nLen);
	void	InsertAlarmEx2Event(ALARM_ALARM_INFO_EX2* pInfo);
	void	InsertPowerFaultEvent(ALARM_POWERFAULT_INFO* pInfo);

	void	InsertEventGuardDetect(ALARM_GUARD_DETECT_INFO* pGuardDectInfo);
	void	InsertEventGuardUpdate(ALARM_GUARD_UPDATE_INFO* pGuardUpdate);

	void	InsertAnalogAlarmEvent(ALARM_ANALOGALARM_EVENT_INFO* pInfo);
    void    InsertAlarmClearEvent(ALARM_ALARMCLEAR_INFO* pInfo);
    void    InsertCIDEvent(ALARM_CIDEVENT_INFO* pInfo);
    void    InsertRCEmergencyCall(ALARM_RCEMERGENCY_CALL_INFO* pInfo);
    void    InsertSubSystemStatusChange(ALARM_SUBSYSTEM_STATE_CHANGE_INFO* pInfo);
    void    InsertModuleLost(ALARM_MODULE_LOST_INFO* pInfo);
    void    InsertPSTNBreakLine(ALARM_PSTN_BREAK_LINE_INFO* pInfo);
    void    InsertBatterPowerInfo(ALARM_BATTERYPOWER_INFO* pInfo);
    void    InserWireLessLowPower(ALARM_WIRELESSDEV_LOWPOWER_INFO* pInfo);

    void    InsertArmModeChange(ALARM_ARMMODE_CHANGE_INFO* pInfo);
    void    InsertBypassModeChange(ALARM_BYPASSMODE_CHANGE_INFO* pInfo);

	void    GetTimeExStringByStruct(NET_TIME_EX stuTime, char *pTime);

private:
    void    ConvertRCEmergencyCallType(EM_RCEMERGENCY_CALL_TYPE emType, CString& csType);
    void    ConvertRCEmergencyModeType(EM_RCEMERGENCY_MODE_TYPE emType, CString& csMode);
    void    ConvertTriggerMode(NET_EM_TRIGGER_MODE emMode, CString& csMode);

private:
	LLONG		m_hLogin;
	int			m_nAlarmIndex;
	UINT32		m_uiAlarmIn;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALARMSUBSCRIBE_H__C7BFC1C5_D321_4CEE_BE90_FB6A67BD954B__INCLUDED_)
