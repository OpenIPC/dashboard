#if !defined(AFX_DLGALARMCHANNEL_H__CCB86EF7_9DC2_4DBA_9CE8_B11799649CC0__INCLUDED_)
#define AFX_DLGALARMCHANNEL_H__CCB86EF7_9DC2_4DBA_9CE8_B11799649CC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAlarmChannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmChannel dialog

class CDlgAlarmChannel : public CDialog
{
// Construction
public:
	CDlgAlarmChannel(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAlarmChannel)
	enum { IDD = IDD_DLG_ALARMCHANNELS };
	CListCtrl	m_lstAlarmInChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarmChannel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarmChannel)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmchannelsBtnQuery();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg();
	BOOL getInfoFromDevice();
	void showInfo();
	
	LLONG	m_lLoginId;
	NET_ALARM_CHANNELS	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARMCHANNEL_H__CCB86EF7_9DC2_4DBA_9CE8_B11799649CC0__INCLUDED_)
