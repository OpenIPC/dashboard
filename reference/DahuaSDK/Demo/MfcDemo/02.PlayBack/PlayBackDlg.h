// ClientDemo3Dlg.h : header file
//
//{{AFX_INCLUDES()
//#include "daytimepicksdk.h"
//}}AFX_INCLUDES

#if !defined(AFX_CLIENTDEMO3DLG_H__5F0C6987_3711_4A4D_8240_CEEC190FD725__INCLUDED_)
#define AFX_CLIENTDEMO3DLG_H__5F0C6987_3711_4A4D_8240_CEEC190FD725__INCLUDED_

#include "../../../Include/Common/dhnetsdk.h"
#include "LockRecord.h"	// Added by ClassView
#include "PlaybackByTime.h"	// Added by ClassView

#include <AFXMT.H>
#include "playbacktimeview.h"
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



/////////////////////////////////////////////////////////////////////////////
// PlayBackDlg dialog
//Playback mode 
enum PlayBackType{
	EM_FILEBACK,
	EM_TIMEBACK,
    EM_MULTYBACK,
};

// 播放速度
enum PlaySpeed
{
    SPEED_MIN,					// 最小速度
    SPEED_DOWN_8= SPEED_MIN,	// 减速到1/8
    SPEED_DOWN_4, 
    SPEED_DOWN_2, 
    SPEED_NORMAL,				// 正常速度
    SPEED_UP_2, 
    SPEED_UP_4, 
    SPEED_UP_8, 				// 加速到8倍 
    SPEED_MAX = SPEED_UP_8,	// 最大速度
};

class CPlayBackDlg : public CDialog,private CPlayBackTimeView::NotifyMovePosition
{
// Construction
public:
	//Download by time
	BOOL DownLoadByTime(int nChannelId, LPNET_TIME tmStart, LPNET_TIME tmEnd, BOOL bLoad);
	BOOL DownLoadByType(int nChannelId, LPNET_TIME tmStart, LPNET_TIME tmEnd, BOOL bLoad,int ConvertType);

	//Playback by time
    //void PlayBackByTime(int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTime);
	void PlayBackByTime(int nChannelID, int nPlayDirection, int nRecordFileType = 0);
	//Interface to callback 	
	void ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize);
	void TimeDownLoadCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO& recordfileinfo,LDWORD dwUser);
	//Callback function when device disconnected.
	friend void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	//Process status callback 
	friend void CALLBACK PlayCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser);
	//Download callback in time mode
	friend void CALLBACK TimeDownLoadPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO& recordfileinfo, LDWORD dwUser);
	//Data callback
	friend int CALLBACK DataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser);
	
	int ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize);
    //void PlayBackByTimeServerMode(int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTime);
	void PlayBackByTimeServerMode(int nChannelID, int nPlayDirection, int nRecordFileType = 0);
	void ExSliderButtonUp();
	void ExSliderButtonDown();

	void AddTimeToDateTimeCtl(NET_RECORDFILE_INFO fileInfo[2000], int nFileCount, int nPlayDirection);
    void AddTimeToDateTimeCtl_Multy(NET_RECORDFILE_INFO fileInfo[2000], int nFileCount);

	void StopPlay();

	void InitTimeCtlBySingleFileInfo(NET_RECORDFILE_INFO & fileInfo);
    
	long m_pDownLoadFile;
	long m_pDownLoadByTime;
	long m_pCurrentTime;
	NET_TIME m_PlayBackByTimeStart;
	NET_TIME m_PlayBackByTimeEnd;
	NET_TIME m_stuBeginTime;
    NET_MULTI_PLAYBACK_PARAM m_MultyPlayBackParam;
	BOOL m_bPlayBackByTimeMode;

	int m_nPlayDirection; // -1未设置, 0正放, 1倒放

	CPlayBackDlg(CWnd* pParent = NULL);	// standard constructor

	virtual void invoke(const COleDateTime* pTime );

// Dialog Data
	//{{AFX_DATA(PlayBackDlg)
	enum { IDD = IDD_CLIENTDEMO3_DIALOG };
//	CExSliderCtrl	m_ctlPlayBack;
	CTabCtrl	m_Tab;
	CIPAddressCtrl	m_ctlDvrIP;
	CString	m_strUserName;
	CString	m_strPwd;
	UINT	m_nPort;

	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PlayBackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(PlayBackDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonStop();
#ifdef _WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#elif defined(_WIN32)
	afx_msg void OnTimer(UINT nIDEvent);
#endif
	afx_msg void OnReleasedcaptureSliderPlayposition(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonSlow();
	afx_msg void OnButtonFast();
	afx_msg void OnButtonNormalspeed();
	afx_msg LRESULT OnSuccess(WPARAM w, LPARAM l);
	afx_msg void OnButtonCapturePicture();	
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	DWORD m_StartTime;

	void BackDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize);

	//int		m_nPlayChannelNum;
private:
    void SetPlayControlButton(BOOL bFlag);
	void ClosePlayBack();
	PlayBackMode m_ePlayMode;
	CTime ConvertNetTimeToCTime(LPNET_TIME lpNetTime);
	PlayBackType m_PlayBackType;
	void LastError();
	UINT m_nTimer;
	DWORD m_dwCurValue;
	DWORD m_TotalTime;
	DWORD m_dwTotalSize;
	LLONG m_hPlayBack;
	int m_nChannelCount;
	INT m_nChannelID;
	BOOL m_bSpeedCtrl;
	void SetDlgState(CWnd *pWnd, BOOL bShow);
	void DoTab(int nTab);
	CPlaybackByTime *m_pTabTimeDlg;
	CLockRecord *m_pTabRecordDlg;

	void InitTabControl();
	void ShowLoginErrorReason(int nError);
	CString GetDvrIP();
	LLONG m_LoginID;
	void InitNetSDK();
    void ChangeSpeedStatus( int nMode );
    long GetUTCTimebyNetTime(const NET_TIME& stTime);
    PlaySpeed	m_eSpeed;
    int m_nReverseMode;

	CPlayBackTimeView* m_pPlaybackTimeCtl;
	CPlayBackTimeView::VideoTimeList m_timeList;

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CStatic m_timeCtlParentView;
	CStatic m_staticTest;
private:
	CStatic m_showPlaybackTime;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTDEMO3DLG_H__5F0C6987_3711_4A4D_8240_CEEC190FD725__INCLUDED_)
