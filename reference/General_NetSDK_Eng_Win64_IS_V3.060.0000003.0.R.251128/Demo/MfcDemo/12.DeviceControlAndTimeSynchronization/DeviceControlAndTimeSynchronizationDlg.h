// DeviceControlAndTimeSynchronizationDlg.h : header file
//

#if !defined(AFX_DeviceControlAndTimeSynchronizationDLG_H__E990B6D9_E6E0_489E_B918_B7BE30B97DF9__INCLUDED_)
#define AFX_DeviceControlAndTimeSynchronizationDLG_H__E990B6D9_E6E0_489E_B918_B7BE30B97DF9__INCLUDED_

#include "../../../Include/Common/dhnetsdk.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDeviceControlAndTimeSynchronizationDlg dialog

class CDeviceControlAndTimeSynchronizationDlg : public CDialog
{
// Construction
public:	
	//Callback function when device disconnected 
	friend void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	friend void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

	CDeviceControlAndTimeSynchronizationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDeviceControlAndTimeSynchronizationDlg)
	enum { IDD = IDD_DeviceControlAndTimeSynchronization_DIALOG };
	CComboBox	m_comboOutChannel;
	CComboBox	m_comboChannel;
	CComboBox	m_comboDisk;
	CComboBox	m_comboOper;
	CIPAddressCtrl	m_dvrIP;
	int		m_dvrPort; // [0, 100000]
	CString	m_strDvrPwd;
	CString	m_strDvrUserName;
	CTime	m_CurTime;
	CTime	m_dvrDate;
	CTime	m_dvrTime;
	COleDateTime	m_CurDate;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceControlAndTimeSynchronizationDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDeviceControlAndTimeSynchronizationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnLogin();
	afx_msg void OnDestroy();
	afx_msg void OnBtnLogout();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonRestart();
	afx_msg void OnButtonDiskctrl();
	afx_msg void OnButtonAlarmstart();
	afx_msg void OnButtonAlarmstop();
	afx_msg void OnButtonOutalarmstart();
	afx_msg void OnButtonOutalarmstop();
	afx_msg void OnButtonNum0();
	afx_msg void OnButtonNum1();
	afx_msg void OnButtonNum2();
	afx_msg void OnButtonNum3();
	afx_msg void OnButtonNum4();
	afx_msg void OnButtonNum5();
	afx_msg void OnButtonNum6();
	afx_msg void OnButtonNum7();
	afx_msg void OnButtonNum8();
	afx_msg void OnButtonNum9();
	afx_msg void OnButtonNum10();
	afx_msg void OnButtonNum11();
	afx_msg void OnButtonNum12();
	afx_msg void OnButtonNum13();
	afx_msg void OnButtonNum14();
	afx_msg void OnButtonNum15();
	afx_msg void OnButtonNum16();
	afx_msg void OnButtonRec();
	afx_msg void OnButtonPrew();
	afx_msg void OnButtonSlow();
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonFast();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonJmpup();
	afx_msg void OnButtonJmpdown();
	afx_msg void OnButtonEsc();
	afx_msg void OnButtonFn1();
	afx_msg void OnButtonFn2();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnButtonLeft();
	afx_msg void OnButtonRight();
	afx_msg void OnButtonEnter();
	afx_msg void OnButtonNine();
	afx_msg void OnButtonOne();
	afx_msg void OnButtonInfo();
	afx_msg void OnButtonAddr();
	afx_msg void OnButtonSplit();
	afx_msg void OnButtonOpenprev();
	afx_msg void OnButtonCloseprev();
	afx_msg void OnButtonPower();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitComBox(int nInChannel,int nOutChannel);
	void GetDiskInfo();
	//Log in handle
	LLONG m_LoginID;
	void ShowLoginErrorReason(int nError);
	CString GetDvrIP();
	void InitNetSDK();
	BOOL ConvertToDateTime(const COleDateTime &date, const CTime &time, NET_TIME &curDateTime);
	int m_nCurDiskType;
	DH_HARDDISK_STATE m_stuDisk;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DeviceControlAndTimeSynchronizationDLG_H__E990B6D9_E6E0_489E_B918_B7BE30B97DF9__INCLUDED_)
