#if !defined(AFX_PLAYBACKBYTIME_H__627B1375_E13B_4173_A10C_D579CE1DCEDE__INCLUDED_)
#define AFX_PLAYBACKBYTIME_H__627B1375_E13B_4173_A10C_D579CE1DCEDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlayBackByTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlayBackByTime dialog

typedef struct _PlayBackByTimeInfo
{
	DeviceNode *pDevice;
	int nChannel;
	NET_TIME starttime;
	NET_TIME endtime;
}PlayBackByTimeInfo;

class CPlayBackByTime : public CDialog
{
// Construction
public:
	CPlayBackByTime(CWnd* pParent = NULL);   // standard constructor

	PlayBackByTimeInfo *GetPlayBackInfo();

// Dialog Data
	//{{AFX_DATA(CPlayBackByTime)
	enum { IDD = IDD_PLAYBYTIME };
	CComboBox	m_devicesel;
	UINT	m_channelsel;
	CTime	m_enddate;
	CTime	m_endtime;
	CTime	m_startdate;
	CTime	m_starttime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayBackByTime)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	
	int InitComboxCallback_Imp(const DeviceNode& node);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlayBackByTime)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	PlayBackByTimeInfo m_playbackinfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYBACKBYTIME_H__627B1375_E13B_4173_A10C_D579CE1DCEDE__INCLUDED_)
