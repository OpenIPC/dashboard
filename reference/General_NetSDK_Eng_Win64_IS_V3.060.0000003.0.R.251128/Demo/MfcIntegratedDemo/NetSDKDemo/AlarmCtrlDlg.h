#if !defined(AFX_ALARMCTRLDLG_H__6DE10DCD_44E8_4830_ABA3_915908E9B71C__INCLUDED_)
#define AFX_ALARMCTRLDLG_H__6DE10DCD_44E8_4830_ABA3_915908E9B71C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlarmCtrlDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAlarmCtrlDlg dialog
#define  MAX_IO_NUM  DH_MAX_ALARM_IN_NUM
class CAlarmCtrlDlg : public CDialog
{
	LLONG m_DeviceId;
	int m_inNum;
	int m_outNum;
	ALARM_CONTROL m_inState[MAX_IO_NUM];
	ALARM_CONTROL m_outState[MAX_IO_NUM];

	ALARMCTRL_PARAM m_triggerIn[MAX_IO_NUM];
// Construction
public:
	CAlarmCtrlDlg(CWnd* pParent = NULL);   // standard constructor

	void SetDeviceId(LLONG nDeviceId);  //设置当前设备ID
// Dialog Data
	//{{AFX_DATA(CAlarmCtrlDlg)
	enum { IDD = IDD_ALARMCTRL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlarmCtrlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAlarmCtrlDlg)
	afx_msg void OnSaveIo();
	afx_msg void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnTrigger();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALARMCTRLDLG_H__6DE10DCD_44E8_4830_ABA3_915908E9B71C__INCLUDED_)
