#if !defined(AFX_DOWNLOADBYTIME_H__8CFBD00A_3F43_40DC_AF06_4E2B64DC30CF__INCLUDED_)
#define AFX_DOWNLOADBYTIME_H__8CFBD00A_3F43_40DC_AF06_4E2B64DC30CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DownloadByTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDownloadByTime dialog

typedef struct _DownloadByTimeInfo
{
	DeviceNode *pDevice;
	int nChannel;
	NET_TIME starttime;
	NET_TIME endtime;
}DownloadByTimeInfo;

class CDownloadByTime : public CDialog
{
// Construction
public:
	CDownloadByTime(CWnd* pParent = NULL);   // standard constructor
	void UpdateDownloadPos(LLONG iHandle, DWORD dwTotal, DWORD dwDownload, int index, NET_RECORDFILE_INFO& recordfileinfo);

	friend void CALLBACK BTDownLoadPos(LLONG lPlayHandle, 
		DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO recordfileinfo, LDWORD dwUser);
	
public:

	int InitComboxCallback_Imp(const DeviceNode& node);

// Dialog Data
	//{{AFX_DATA(CDownloadByTime)
	enum { IDD = IDD_DOWNLOADBYTIME };
	CComboBox	m_devicesel;
	CProgressCtrl	m_download_pos;
	UINT	m_channelsel;
	CTime	m_enddate;
	CTime	m_endtime;
	CTime	m_startdate;
	CTime	m_starttime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDownloadByTime)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDownloadByTime)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnDownload();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LLONG m_pdownloadfile;
	DWORD m_downloadtotal;
	DWORD m_downloadsize;
	BOOL  m_bInited;
	CString m_strIdx;
	CString m_strPos;
	CString m_strFileInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOWNLOADBYTIME_H__8CFBD00A_3F43_40DC_AF06_4E2B64DC30CF__INCLUDED_)
