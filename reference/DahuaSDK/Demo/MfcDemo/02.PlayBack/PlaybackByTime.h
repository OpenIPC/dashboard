#if !defined(AFX_PLAYBACKBYTIME_H__35EAB30F_C6FB_4466_84AC_BF1C76B9623E__INCLUDED_)
#define AFX_PLAYBACKBYTIME_H__35EAB30F_C6FB_4466_84AC_BF1C76B9623E__INCLUDED_

#include "../../../Include/Common/dhnetsdk.h"
#include "LockRecord.h"
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//

/////////////////////////////////////////////////////////////////////////////
// CPlaybackByTime dialog

//Search type
enum QueryType{
	EM_LISTALL,
	EM_ALARMLOCAL,
	EM_VIDEOMOTION,
};

//Play mode 
enum PlayBackMode{
	EM_DIRECTMODE,
	EM_SERVERMODE,
};

//Stream type 
enum PlayBackStream{
	EM_BOTH_STREAM,
	EM_MAIN_STREAM,
	EM_EXTRA_STREAM
};

//Play direction
enum PlayBackDirection{
	EM_FORWARD,
	EM_BACKWARD
};

class CPlaybackByTime : public CDialog
{
// Construction
public:
	//Set channel dropdown menu 
	void InitComboBox(int nChannel,LLONG lLoginID);
	//Log out and then clear
	void Clean();

	long m_hLoginID;
	DWORD m_dwCurValue;
	DWORD m_dwTotalSize;
	CPlaybackByTime(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlaybackByTime)
	enum { IDD = IDD_PLAYBACK_BYTIME };
	CComboBox	m_ctlStreamType;
	CComboBox	m_ctlQueryType;
	CComboBox	m_ctlPlayMode;
	CComboBox m_ctlPlayDirection;
	CComboBox	m_ctlLoadChannel;
	CProgressCtrl	m_ctlLoadPro;
	CComboBox	m_ctlChannel;
	CTime	m_timeLoadFrom;
	CTime	m_timeLoadTo;
	COleDateTime	m_dateLoadFrom;
	COleDateTime	m_dateLoadTo;
	CComboBox	m_comboConvertType;

	int	m_nPlayDirection;	// 控制播放方向, -1 未设置， 0 正放， 1 倒放
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaybackByTime)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlaybackByTime)
	afx_msg void OnButtonDownload();
	virtual BOOL OnInitDialog();
#ifdef _WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#elif defined(_WIN32)
	afx_msg void OnTimer(UINT nIDEvent);
#endif
	afx_msg void OnSelectMonthcalendar(NMHDR* pNMHDR, LRESULT* pResult);
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void OnButtonPlay(int nPlayDirection);
private:
	UINT m_nTimer;
	NET_RECORDFILE_INFO m_netFileInfo[2000];
	NET_TIME ConvertTime(CTime date,CTime time);
	NET_TIME ConvertToDateTime(const COleDateTime& date, const CTime& time);
	/// NET_TIME结构体之间的大小比较
	int Compare(const NET_TIME *pSrcTime, const NET_TIME *pDestTime);
    void SetControlButton(BOOL nFlag);

public:
	afx_msg void OnCbnSelchangeComboPlayReverse();
	afx_msg void OnBnClickedButtonDownload2();
	afx_msg void OnCbnSelchangeComboLoadchannel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYBACKBYTIME_H__35EAB30F_C6FB_4466_84AC_BF1C76B9623E__INCLUDED_)
