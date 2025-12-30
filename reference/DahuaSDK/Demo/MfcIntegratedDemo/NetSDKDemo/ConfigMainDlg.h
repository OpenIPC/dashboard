#if !defined(AFX_CONFIGMAINDLG_H__60D08369_F40C_4DA5_A6F1_9674D5378BA0__INCLUDED_)
#define AFX_CONFIGMAINDLG_H__60D08369_F40C_4DA5_A6F1_9674D5378BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigMainDlg.h : header file
//

#include "ConfigServer.h"
#include "ConfigComm.h"
#include "ConfigChannel.h"
#include "ConfigAlarm.h"
#include "ConfigRecord.h"
#include "ConfigNetwork.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigMainDlg dialog

class CConfigMainDlg : public CDialog
{
// Construction
public:
	CConfigMainDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigMainDlg)
	enum { IDD = IDD_CONFIG_MAIN };
	CTabCtrl	m_configBoard;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigMainDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigMainDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTabBoard(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SetDevice(DeviceNode *pDev);
private:
	DeviceNode *m_pDev;

	CConfigServer	m_serverDlg;
	CConfigComm		m_commDlg;
	CConfigChannel	m_channelDlg;
	CConfigAlarm	m_alarmDlg;
	CConfigRecord	m_recordDlg;
	CConfigNetwork	m_networkDlg;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGMAINDLG_H__60D08369_F40C_4DA5_A6F1_9674D5378BA0__INCLUDED_)
