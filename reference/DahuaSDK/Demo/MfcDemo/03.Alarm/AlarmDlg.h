// AlarmDlg.h : header file
//

#if !defined(AFX_AlarmDLG_H__7F279BB2_041F_4199_9E49_32FC11D6CD45__INCLUDED_)
#define AFX_AlarmDLG_H__7F279BB2_041F_4199_9E49_32FC11D6CD45__INCLUDED_

#include <afxtempl.h>
#include <afxmt.h>
#include <math.h>

#include "QueryAlarmState.h"
#include "AlarmInfo.h"
#include "DevAlarmDlg.h"
#include "../../../Include/Common/dhnetsdk.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAlarmDlg dialog

class CAlarmDlg : public CDialog
{
// Construction
public:
	//Callback interface 
	//callback function when device disconnected 
	friend void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	//callback function when device reconnect 
	friend void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	//Message callback process function. It is a callback to the whole SDK implement to process the callbacked alarm message.
	friend BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

	//Log in handle 
	LLONG m_LoginID;
	CCriticalSection m_cs;

	CAlarmDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAlarmDlg)
	enum { IDD = IDD_Alarm_DIALOG };
	CIPAddressCtrl	m_dvrIP;
	CTabCtrl		m_Tab;
	int				m_dvrPort; // [0, 100000]
	CString			m_strDvrUserName;
	CString			m_strDvrPwd;
	SYSTEMTIME		stuTime;
	SYSTEMTIME		stuTimeLater;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlarmDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAlarmDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bIsListen;
	CList<CAlarmInfoEx, CAlarmInfoEx&> m_listAlarmInfo;	
	
	CQueryAlarmState*   m_pTabQueryAlarm;
    CDevAlarmDlg*       m_pTabDevAlarmDlg;

	void ShowLoginErrorReason(int nError);
	CString GetDvrIP();
	void InitNetSDK();
	void InitTabControl();
	void DoTab(int nTab);
	void SetDlgState(CWnd *pWnd,BOOL bShow);

    // WM_DEVICE_ALARM message function
	bool FindInCList(int nAlarmType, int nChannel, POSITION& pos);
    LRESULT	OnDevAlarm(WPARAM wParam, LPARAM lParam);
	void UpdateAlarmListCtrl(const CAlarmInfoEx& alarmInfo);
	void ClearAlarmInfoList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AlarmDLG_H__7F279BB2_041F_4199_9E49_32FC11D6CD45__INCLUDED_)
