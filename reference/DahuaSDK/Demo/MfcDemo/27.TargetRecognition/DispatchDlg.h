#if !defined(AFX_DISPATCHDLG_H__822D0E65_92E2_48B4_92DA_CC0BBC76665A__INCLUDED_)
#define AFX_DISPATCHDLG_H__822D0E65_92E2_48B4_92DA_CC0BBC76665A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DispatchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDispatchDlg dialog

class CDispatchDlg : public CDialog
{
// Construction
public:
	CDispatchDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDispatchDlg)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDispatchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDispatchDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPATCHDLG_H__822D0E65_92E2_48B4_92DA_CC0BBC76665A__INCLUDED_)
