#if !defined(AFX_MYLISTCTRL_H__A2E0BBD0_7257_43B2_A124_A901CA1AAAEC__INCLUDED_)
#define AFX_MYLISTCTRL_H__A2E0BBD0_7257_43B2_A124_A901CA1AAAEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MylistCtrl.h : header file
//
#include <afxtempl.h>    

/////////////////////////////////////////////////////////////////////////////
// CMylistCtrl window

typedef struct      
{    
    COLORREF colText;    
    COLORREF colTextBk;    
}TEXT_BK;    


class CMylistCtrl : public CListCtrl
{
// Construction
public:
	CMylistCtrl();

// Attributes
public:
	//Set a line foreground color and background color 
    void SetItemColor(DWORD iItem, COLORREF TextColor, COLORREF TextBkColor);     
    //set all lines foreground color and background color
	void SetAllItemColor(DWORD iItem, COLORREF TextColor, COLORREF TextBkColor);   
    //clear color
	void ClearColor();                                                            


// Operations
public:
	CMap<DWORD, DWORD, TEXT_BK, TEXT_BK> MapItemColor;	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMylistCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMylistCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMylistCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
    void CMylistCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);    
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYLISTCTRL_H__A2E0BBD0_7257_43B2_A124_A901CA1AAAEC__INCLUDED_)
