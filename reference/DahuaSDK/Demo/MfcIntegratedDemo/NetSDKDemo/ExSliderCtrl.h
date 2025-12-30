#if !defined(AFX_EXSLIDERCTRL_H__40BB1F1E_2542_4440_BD98_DAE14E43D8C2__INCLUDED_)
#define AFX_EXSLIDERCTRL_H__40BB1F1E_2542_4440_BD98_DAE14E43D8C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExSliderCtrl window

class CExSliderCtrl : public CSliderCtrl
{
// Construction
public:
	CExSliderCtrl();

// Attributes
public:
	BOOL m_bLButtonDown;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExSliderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CExSliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CExSliderCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXSLIDERCTRL_H__40BB1F1E_2542_4440_BD98_DAE14E43D8C2__INCLUDED_)
