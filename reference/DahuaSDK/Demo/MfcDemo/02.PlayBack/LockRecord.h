#if !defined(AFX_LOCKRECORD_H__9F2433DE_CF15_42D1_AB9E_AF306B0471EB__INCLUDED_)
#define AFX_LOCKRECORD_H__9F2433DE_CF15_42D1_AB9E_AF306B0471EB__INCLUDED_

#include "../../../Include/Common/dhnetsdk.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LockRecord.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLockRecord dialog

class CLockRecord : public CDialog
{
// Construction
public:
	DWORD m_dwTotalSize;
	DWORD m_dwCurValue;

	//Set channel 
	void InitComboBox(int nChannel,LLONG lLoginID);
	//Log off and then clear
	void Clean();
	
	CLockRecord(CWnd* pParent = NULL);   // standard constructor

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLockRecord)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLockRecord)
	virtual BOOL OnInitDialog();
#ifdef _WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#elif defined(_WIN32)
	afx_msg void OnTimer(UINT nIDEvent);
#endif
	afx_msg void OnButtonLock();
	afx_msg void OnButtonUnlock();
	afx_msg void OnButtonQuery();
	afx_msg void OnButtonCleanlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// Dialog Data
	//{{AFX_DATA(CLockRecord)
	enum { IDD = IDD_LOCKRECORD };
	CComboBox	m_ctlChannel;
	CTime	m_timeFrom;
	CTime	m_timeTo;
	COleDateTime	m_dateFrom;
	COleDateTime	m_dateTo;
	CListCtrl	m_ctlListInfo;
	//}}AFX_DATA

	LLONG m_lLoginID;
	BOOL m_bUnLockQueryFlag;
	UINT m_nTimer;
	NET_RECORDFILE_INFO m_netFileInfo[2000];

	void AddRecordInfoToListView(LPNET_RECORDFILE_INFO pRecordFiles,int nFileCount);
	NET_TIME ConvertTime(CTime date,CTime time);
	NET_TIME ConvertToDateTime(const COleDateTime &date, const CTime &time);
	void InitListView();
    void SetControlButton(BOOL bFlag);	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCKRECORD_H__9F2433DE_CF15_42D1_AB9E_AF306B0471EB__INCLUDED_)
