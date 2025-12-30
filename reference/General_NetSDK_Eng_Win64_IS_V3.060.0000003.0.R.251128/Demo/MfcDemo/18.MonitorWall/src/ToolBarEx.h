#if !defined(AFX_TOOLBAREX_H__E32E42BB_E4C2_4A68_B1AC_FD92C2B615CB__INCLUDED_)
#define AFX_TOOLBAREX_H__E32E42BB_E4C2_4A68_B1AC_FD92C2B615CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarEx.h : header file
//
#include "afxtempl.h"
/////////////////////////////////////////////////////////////////////////////
// CToolBarEx window

class CToolBarEx : public CToolBar
{
// Construction
public:
	CToolBarEx();

// Attributes
public:

// Operations
public:
	BOOL SetButtonTitle(UINT nID, LPCTSTR lpszText);
	BOOL EnableButton(UINT nID, BOOL bEnable = TRUE);
	void ShowButton(UINT nID, BOOL bShow = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolBarEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolBarEx)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CWnd* GetTitleWnd(UINT nID)
	{
		CWnd* pWnd = NULL;
		m_mapTitleWnd.Lookup(nID, pWnd);	
		return pWnd;
	}

protected:
	CMap<UINT, UINT, CWnd*, CWnd*> m_mapTitleWnd;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBAREX_H__E32E42BB_E4C2_4A68_B1AC_FD92C2B615CB__INCLUDED_)
