#if !defined(AFX_AUDIOBROADCASTDLG_H__FE386961_BC18_41CE_9EAC_070CF2F9FCAC__INCLUDED_)
#define AFX_AUDIOBROADCASTDLG_H__FE386961_BC18_41CE_9EAC_070CF2F9FCAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioBroadcastDlg.h : header file
//
#include "NetSDKDemo.h"
/////////////////////////////////////////////////////////////////////////////
// CAudioBroadcastDlg dialog

class CAudioBroadcastDlg : public CDialog
{
// Construction
public:
	CAudioBroadcastDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAudioBroadcastDlg)
	enum { IDD = IDD_AUDIOBORADCAST };
	CListCtrl	m_broadcastDevListCtrl;
	CListCtrl				m_allDevListCtrl;
	BOOL	m_bRecord;
	//}}AFX_DATA

	list<DeviceNode *>		m_AllDevList;
	list<DeviceNode *>		m_BroadcastDevList;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioBroadcastDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAudioBroadcastDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAdd();
	afx_msg void OnAddall();
	afx_msg void OnDel();
	afx_msg void OnDelall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void MoveDevAll2Bc(int index);
	void MoveDevBc2All(int index);
	void UpDataDevList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOBROADCASTDLG_H__FE386961_BC18_41CE_9EAC_070CF2F9FCAC__INCLUDED_)
