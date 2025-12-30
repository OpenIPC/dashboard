#if !defined(AFX_ALARMCONFIG_H__C6F7BB46_EDDD_4AAB_A0DF_9E63CBC1087D__INCLUDED_)
#define AFX_ALARMCONFIG_H__C6F7BB46_EDDD_4AAB_A0DF_9E63CBC1087D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlarmConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAlarmConfig dialog

class CAlarmConfig : public CDialog
{
// Construction
public:
	CAlarmConfig(CWnd* pParent = NULL);   // standard constructor

	BOOL bSetAlarmParams;

	void SetDevice(DeviceNode *nDev);

	DWORD dwReturned;

	BOOL F_TimeTest();
// Dialog Data
	//{{AFX_DATA(CAlarmConfig)
	enum { IDD = IDD_CHILD_ALARMCFG };
	CComboBox	m_copyoutsel;
	CComboBox	m_copyalarminsel;
	CComboBox	m_copyalarmdaysel;
	CComboBox	m_alarmtypesel;
	CComboBox	m_alarmouttimesel;
	CComboBox	m_alarmoutsel;
	CComboBox	m_alarmdaysel;
	CComboBox	m_alarminsel;
	BOOL	m_bAlarmIn;
	CString	m_alarmname;
	UINT	m_hour1;
	UINT	m_hour2;
	UINT	m_hour3;
	UINT	m_hour4;
	UINT	m_minute1;
	UINT	m_minute2;
	UINT	m_minute3;
	UINT	m_minute4;
	UINT	m_shour1;
	UINT	m_shour2;
	UINT	m_shour3;
	UINT	m_shour4;
	UINT	m_sminute1;
	UINT	m_sminute3;
	UINT	m_sminute4;
	UINT	m_sminute2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlarmConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAlarmConfig)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmapply();
	afx_msg void OnSelchangeAlarminsel();
	afx_msg void OnAlarmoutrestore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DeviceNode *m_Device;

	NET_DEV_ALARMINCFG m_alarmincfg[MAX_ALARMIN];
	BOOL m_bGetAlarmIn[MAX_ALARMIN];
	NET_DEV_ALARMOUTCFG m_alarmoutcfg[MAX_ALARMOUT];
	BOOL m_bGetAlarmOut[MAX_ALARMOUT];
	int m_alarmin;
	int m_alarmout;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALARMCONFIG_H__C6F7BB46_EDDD_4AAB_A0DF_9E63CBC1087D__INCLUDED_)
