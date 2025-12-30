#if !defined(AFX_CONFIGALARM_H__02D8B6C8_E0DD_4C05_9EDB_9724E1AAD59E__INCLUDED_)
#define AFX_CONFIGALARM_H__02D8B6C8_E0DD_4C05_9EDB_9724E1AAD59E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarm.h : header file
//

#include "ConfigAlarmAlarm.h"
#include "ConfigAlarmBlind.h"
#include "ConfigAlarmDisk.h"
#include "ConfigAlarmMotion.h"
#include "ConfigAlarmVlost.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarm dialog

class CConfigAlarm : public CDialog
{
// Construction
public:
	CConfigAlarm(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarm)
	enum { IDD = IDD_CONFIG_ALARM };
	CTabCtrl	m_alarmBoard;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarm)
	afx_msg void OnSelchangeTabAlarmboard(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUndoAll();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetConfig();
	void SetDevice(DeviceNode *pDev);
	
private:
	DeviceNode *m_pDev;
	DHDEV_ALARM_SCHEDULE m_almCFG;
	BOOL m_bInited;

	CConfigAlarmAlarm	m_alarmInDlg;
	CConfigAlarmBlind	m_blindDlg;
	CConfigAlarmDisk	m_diskDlg;
	CConfigAlarmMotion	m_motionDlg;
	CConfigAlarmVlost	m_vlostDlg;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARM_H__02D8B6C8_E0DD_4C05_9EDB_9724E1AAD59E__INCLUDED_)
