// PassengerFlowDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


class CAlarmSubscribe;
class CRealAttch;

// CPassengerFlowDlg dialog
class CPassengerFlowDlg : public CDialog
{
// Construction
public:
	CPassengerFlowDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PASSENGERFLOW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	static void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	static void CALLBACK HaveReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	static void CALLBACK VideoStatSumCallBackFunc(LLONG lAttachHandle, NET_VIDEOSTAT_SUMMARY* pBuf, DWORD dwBufLen, LDWORD dwUser);

private:
	void	ShowLoginErrorReason(int nError);
	void	VideoStatSummary(NET_VIDEOSTAT_SUMMARY* pSummaryInfo);
	void	InitControlTab();
	void	DoTab(int nTab);
	void	SetDlgState(CWnd *pWnd, BOOL bShow);

	void	InitNetSDK();
	void	UnInitNetSDK();

private:
	LLONG	m_loginID;
	int		m_nChannelNum;
	LLONG	m_lPlayID;
	LLONG	m_lAttachID;

private:
	CAlarmSubscribe *m_pTabAlarmDlg;
	CRealAttch		*m_pTabRealAttchDlg;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnHistoryQuery();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnLogout();
	afx_msg void OnDestroy();

	LRESULT OnDeviceDisConnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeviceReconnect(WPARAM wParam, LPARAM lParam);

protected:
	CComboBox m_cbxChannel;

public:
	CTabCtrl m_Tab;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeCbxChannel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
