#if !defined(AFX_TIMINGRECORD_H__699B1928_50B0_4ECB_B000_E7B0F3BBE011__INCLUDED_)
#define AFX_TIMINGRECORD_H__699B1928_50B0_4ECB_B000_E7B0F3BBE011__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimingRecord.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTimingRecord dialog

class CTimingRecord : public CDialog
{
// Construction
public:
	CTimingRecord(CWnd* pParent = NULL);   // standard constructor

	SetRecordInfo(NET_DEV_RECORD *recinfo);

	void EnableControl(BOOL bEnable);
	BOOL TimeTest();
// Dialog Data
	//{{AFX_DATA(CTimingRecord)
	enum { IDD = IDD_TIMINGRECORD };
	CComboBox	m_alldaytypesel;
	CComboBox	m_recordtype4sel;
	CComboBox	m_recordtype3sel;
	CComboBox	m_recordtype2sel;
	CComboBox	m_recordtype1sel;
	CComboBox	m_recorddaysel;
	CComboBox	m_copydaysel;
	BOOL	m_bAllDayRecord;
	UINT	m_hour1;
	UINT	m_hour2;
	UINT	m_hour3;
	UINT	m_hour4;
	UINT	m_minute1;
	UINT	m_minute2;
	UINT	m_minute3;
	UINT	m_minute4;
	UINT	m_shour1;
	UINT	m_shour2;
	UINT	m_shour3;
	UINT	m_shour4;
	UINT	m_sminute1;
	UINT	m_sminute2;
	UINT	m_sminute3;
	UINT	m_sminute4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimingRecord)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NET_DEV_RECORD *m_recordinfo;
	int m_nDate;
	int m_nCopyDate;

	// Generated message map functions
	//{{AFX_MSG(CTimingRecord)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeRecorddaysel();
	afx_msg void OnAlldayCheck();
	afx_msg void OnBtnok();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMINGRECORD_H__699B1928_50B0_4ECB_B000_E7B0F3BBE011__INCLUDED_)
