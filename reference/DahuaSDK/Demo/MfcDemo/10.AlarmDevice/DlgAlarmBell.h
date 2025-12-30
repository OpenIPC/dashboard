#if !defined(AFX_DLGALARMBELL_H__9B5D7B71_B7D7_4321_A26F_1B53185C218A__INCLUDED_)
#define AFX_DLGALARMBELL_H__9B5D7B71_B7D7_4321_A26F_1B53185C218A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAlarmBell.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmBell dialog

class CDlgAlarmBell : public CDialog
{
// Construction
public:
	CDlgAlarmBell(CWnd* pParent = NULL, LLONG hLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAlarmBell)
	enum { IDD = IDD_DLG_ALARMBELL };
	CListBox	m_lsErrorInfo;
	CComboBox	m_cmbChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarmBell)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarmBell)
	virtual BOOL OnInitDialog();
	afx_msg void OnDlgAlarmbellBtnStart();
	afx_msg void OnDlgAlarmbellBtnStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	OutputInfo(const CString& csInfo);

private:
	LLONG		m_hLogin;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARMBELL_H__9B5D7B71_B7D7_4321_A26F_1B53185C218A__INCLUDED_)
