#if !defined(AFX_PTZSCREEN_H__E554F905_77BB_46C1_B0AC_DABD656755CE__INCLUDED_)
#define AFX_PTZSCREEN_H__E554F905_77BB_46C1_B0AC_DABD656755CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BSWndContainer.h"
#include "playWnd.h"
// PtzScreen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPtzScreen window
#define DH_MAX_CHANNUM 16
//Window split type 
enum{
	SPLIT1 = 0,
	SPLIT4,
	SPLIT9,
	SPLIT16,	
	SPLIT_TOTAL
};

typedef void (CALLBACK *OnMessageProcFunc)(int nWndID, UINT message, LDWORD dwUser);
typedef BOOL (CALLBACK *OnGetParamsFunc)(int nWndID, int type, LDWORD dwUser);
typedef void (CALLBACK *OnSetParamsFunc)(int nWndID, int type, LDWORD dwUser);
typedef void (CALLBACK *OnRectEventFunc)(RECT WinRect,CPoint &pointStart,CPoint  &pointEnd,LDWORD dwUser);

class CPtzScreen : public CBSWndContainer
{
// Construction
public:
	CPtzScreen();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPtzScreen)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPtzScreen();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPtzScreen)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnIconEraseBkgnd(CDC* pDC);
	afx_msg void OnAskCbFormatName(UINT nMaxCount, LPTSTR lpszString);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnCancelMode();
	afx_msg void OnPaint();
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	
	void	SetCallBack(OnMessageProcFunc cbMessageProc, LDWORD dwMessageUser,
		OnGetParamsFunc cbGetParams, LDWORD dwGetParams, 
		OnSetParamsFunc cbSetParams, LDWORD dwSetParmas,
		OnRectEventFunc cbEventParams,LDWORD	dwRectEventParams);
	int		SetShowPlayWin(int nMain, int nSub=0);
	CWnd*	GetPage(int nIndex);
	void    SetActiveWnd(int nIndex,BOOL bRepaint=TRUE);
	
	
public:
	OnMessageProcFunc	m_pMessageProc;
	LDWORD				m_dwMessageUser;
	OnGetParamsFunc		m_pGetParams;
	LDWORD				m_dwGetParams;
	OnSetParamsFunc		m_pSetParams;
	LDWORD				m_dwSetParams;
	OnRectEventFunc     m_pRectEventFunc;
	LDWORD				m_dwRectEvent;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// private member for inter user
private:
	
	CPlayWnd m_wndVideo[DH_MAX_CHANNUM];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PTZSCREEN_H__E554F905_77BB_46C1_B0AC_DABD656755CE__INCLUDED_)
