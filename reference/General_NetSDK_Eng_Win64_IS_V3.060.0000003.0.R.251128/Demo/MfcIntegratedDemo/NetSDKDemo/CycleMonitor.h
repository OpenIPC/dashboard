#if !defined(AFX_CYCLEMONITOR_H__8999856F_8622_452B_A415_DDA705CA4227__INCLUDED_)
#define AFX_CYCLEMONITOR_H__8999856F_8622_452B_A415_DDA705CA4227__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CycleMonitor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCycleMonitor dialog

#include "CycleChild.h"

class CCycleMonitor : public CDialog
{
// Construction
	LLONG m_deviceID;
	int m_curScreen;
	int m_channelnum;

	CCycleChild m_cyclechild[DH_MAX_CHANNUM];
	int m_curRow;//用于记录当前列表末尾位置的下一位置

	int m_curChild;//用于记录当前属性窗口的子窗口

public:
	CCycleMonitor(CWnd* pParent = NULL);   // standard constructor
	
	void DeleteItem(int rightindex);
	void AddItem(int lindex, int rindex);
	// Dialog Data
	//{{AFX_DATA(CCycleMonitor)
	enum { IDD = IDD_CYCLE_MONITOR };
	CListCtrl	m_channellist;
	CTabCtrl	m_ScreenTabControl;
	UINT	m_cycleinterval;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCycleMonitor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:

	int InitLeftTreeCallBack_Imp(const DeviceNode& node);	


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCycleMonitor)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeScreenTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAdd1();
	afx_msg void OnAddall();
	afx_msg void OnDelete1();
	afx_msg void OnDeleteall();
	virtual void OnCancel();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CYCLEMONITOR_H__8999856F_8622_452B_A415_DDA705CA4227__INCLUDED_)
