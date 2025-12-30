#if !defined(AFX_EDITGROUPDLG_H__671184BC_DE10_4103_9861_FFF6B8DBE549__INCLUDED_)
#define AFX_EDITGROUPDLG_H__671184BC_DE10_4103_9861_FFF6B8DBE549__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditGroupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditGroupDlg dialog

class CEditGroupDlg : public CDialog
{
// Construction
public:
	CEditGroupDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditGroupDlg)
	enum { IDD = IDD_EDIT_GROUP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditGroupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditGroupDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITGROUPDLG_H__671184BC_DE10_4103_9861_FFF6B8DBE549__INCLUDED_)
