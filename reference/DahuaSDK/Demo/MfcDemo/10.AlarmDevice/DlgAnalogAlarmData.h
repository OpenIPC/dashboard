#if !defined(AFX_DLGANALOGALARMDATA_H__CDC33ECD_7719_4DF2_BB34_5AC3B13FC640__INCLUDED_)
#define AFX_DLGANALOGALARMDATA_H__CDC33ECD_7719_4DF2_BB34_5AC3B13FC640__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAnalogAlarmData.h : header file
//

#define WM_ON_ANALOGALARM_DATA	(WM_USER + 300)

#define MAX_INFO_SHOW			(1000)

/////////////////////////////////////////////////////////////////////////////
// CDlgAnalogAlarmData dialog

class CDlgAnalogAlarmData : public CDialog
{
// Construction
public:
	CDlgAnalogAlarmData(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAnalogAlarmData)
	enum { IDD = IDD_DLG_ANALOGALARM_DATA };
	CListCtrl	m_lstDataInfo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAnalogAlarmData)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAnalogAlarmData)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnAnalogalarmDataBtnAttach();
	afx_msg void OnAnalogalarmDataBtnDetach();
	afx_msg LRESULT OnAnalogAlarmData(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void CALLBACK AnalogAlarmDataCallBack(LLONG lLoginID, LLONG lAttachHandle, NET_ANALOGALARM_SENSE_INFO* pInfo, int nBufLen);

private:
	void	InitListControl();
	void	GetTimeStringByStruct(NET_TIME stuTime, char *pTime);

	void	UpdateList();
	void	InsertEventInfo(const NET_ANALOGALARM_SENSE_INFO& stuInfo);

private:
	LLONG			m_lLoginID;
	LLONG			m_lAttachID;
	int				m_nIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGANALOGALARMDATA_H__CDC33ECD_7719_4DF2_BB34_5AC3B13FC640__INCLUDED_)
