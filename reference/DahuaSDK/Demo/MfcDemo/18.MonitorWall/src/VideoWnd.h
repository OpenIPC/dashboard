#if !defined(AFX_VIDEOWND_H__EC274283_CDEF_4995_97AD_6A8902C5D9CD__INCLUDED_)
#define AFX_VIDEOWND_H__EC274283_CDEF_4995_97AD_6A8902C5D9CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoWnd.h : header file
//
#define SCREEN_BACK_COLOR			(GetSysColor(COLOR_BTNFACE))
#define ACTIVE_COLOR_TOP_LEFT		RGB(128, 255, 64)
#define ACTIVE_COLOR_RIGHT_BOTTOM	RGB(128, 255, 64)
#define INACTIVE_COLOR_TOP_LEFT		RGB(222, 222, 222)
#define INACTIVE_COLOR_RIGHT_BOTTOM	RGB(222, 222, 222)
#define HEADER_COLOR				RGB(150,150,150)

/////////////////////////////////////////////////////////////////////////////
// CVideoWnd view

class CVideoWnd : public CWnd
{
public:
	CVideoWnd();           // protected constructor used by dynamic creation
	virtual ~CVideoWnd();
	DECLARE_DYNCREATE(CVideoWnd)

// Attributes
public:
	void SetWndID(int nID) { m_nWndID = nID; }
	int  GetWndID(void) const { return m_nWndID; }

	void SetActive(BOOL bActive = TRUE);
	BOOL IsActive() const { return m_bActive; }

	CRect GetHeaderRect() const { return m_rcHeader; }
	CRect GetResizeRect() const { return m_rcResize; }

	BOOL IsResizeEnable() const { return m_bEnableResize; }
	void EnableResize(BOOL bEnable = TRUE) { m_bEnableResize = bEnable; }

	BOOL IsCloseBtnVisible() const { return m_bShowCloseBtn; }
	void ShowCloseBtn(BOOL bVisible = TRUE);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoWnd)
protected:
	virtual	LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Implementation
public:	
	static SIZE MinSize;
protected:
	int	m_nWndID;
	BOOL m_bActive;
	BOOL m_bEnableResize;
	BOOL m_bShowCloseBtn;
	CRect m_rcBtnClose;
	CRect m_rcHeader;
	CRect m_rcContext;
	CRect m_rcBtnClear;
	CRect m_rcResize;

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoWnd)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOWND_H__EC274283_CDEF_4995_97AD_6A8902C5D9CD__INCLUDED_)
