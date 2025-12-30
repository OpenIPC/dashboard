#if !defined(AFX_SPLICESCREENCONFIGDLG_H__24356D35_6D07_4610_8B47_06E72836412D__INCLUDED_)
#define AFX_SPLICESCREENCONFIGDLG_H__24356D35_6D07_4610_8B47_06E72836412D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpliceScreenConfigDlg.h : header file
//
#include "MonitorWallWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CSpliceScreenConfigDlg dialog

class CSpliceScreenConfigDlg : public CDialog
{
// Construction
public:
	CSpliceScreenConfigDlg(LLONG lLoginID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpliceScreenConfigDlg)
	enum { IDD = IDD_DIALOG_SPLICESCREEN };
	CCheckListBox	m_lbOutputs;
	CComboBox	m_cbTVSizeRow;
	CComboBox	m_cbTVSizeColumn;
	CComboBox	m_cbTVLayoutColumn;
	CComboBox	m_cbTVLayoutRow;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpliceScreenConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	InitUI();
	void	EnableOutputs(const WMBlock* pBlockActive);
	void	EnableOutput(int nChannel, BOOL bEnable = TRUE);
	void	CheckOutput(int nChannel, BOOL bCheck = TRUE);

	CMonitorWallWnd	m_wndWall;
	LLONG m_lLoginID;


	// Generated message map functions
	//{{AFX_MSG(CSpliceScreenConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeComboOutputLayoutOrSize();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnBtnClear();
	afx_msg void OnRadioLayout();
	afx_msg void OnRadioSize();
	afx_msg void OnKillfocusEditSpliceName();
	afx_msg void OnCheckChangeOutputs();
	afx_msg void OnKillfocusEditSpliceId();
	afx_msg void OnChangeEditSpliceId();
	afx_msg void OnChangeEditSpliceName();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		LRESULT DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLICESCREENCONFIGDLG_H__24356D35_6D07_4610_8B47_06E72836412D__INCLUDED_)
