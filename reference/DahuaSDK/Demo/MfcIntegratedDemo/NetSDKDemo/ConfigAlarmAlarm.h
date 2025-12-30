#if !defined(AFX_CONFIGALARMALARM_H__91A49E2F_A661_4915_96A2_193AA17B3C68__INCLUDED_)
#define AFX_CONFIGALARMALARM_H__91A49E2F_A661_4915_96A2_193AA17B3C68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarmAlarm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmAlarm dialog

class CConfigAlarmAlarm : public CDialog
{
// Construction
public:
	CConfigAlarmAlarm(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarmAlarm)
	enum { IDD = IDD_CONFIG_ALARM_ALARM };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarmAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarmAlarm)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboAlarmin();
	afx_msg void OnSelchangeComboAlarmweek();
	afx_msg void OnSelchangeComboAlarmptzchl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetDevice(DeviceNode *pDev);
	void SetAlarmInCFG(DH_ALARMIN_CFG *cfg);
	void GetAlarmInCFG(DH_ALARMIN_CFG *cfg);
	void ShowConfig();
	void UndoAll();

private:
	DeviceNode *m_pDev;
	DH_ALARMIN_CFG m_alarmCFG[DH_MAX_ALARM_IN_NUM];
	BOOL m_bReady;
	int m_almIdx;
	int m_weekIdx;
	int m_ptzchlIdx;
	int m_dontsave;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARMALARM_H__91A49E2F_A661_4915_96A2_193AA17B3C68__INCLUDED_)
