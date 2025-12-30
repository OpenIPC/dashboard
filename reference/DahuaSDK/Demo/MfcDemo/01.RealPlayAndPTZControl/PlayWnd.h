#if !defined(AFX_PLAYWND_H__158FCA3F_D545_4DB6_9946_0FC9F7D9D5CE__INCLUDED_)
#define AFX_PLAYWND_H__158FCA3F_D545_4DB6_9946_0FC9F7D9D5CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlayWnd.h : header file
//

#include "MessageText.h"

/////////////////////////////////////////////////////////////////////////////
// CPlayWnd window

/////////////////////////
//	POPUP MENU ID DEFINE

#define		VIDEO_MENU_BASE				WM_USER + 1979
#define		VIDEO_MENU_END				WM_USER + 1985
#define		VIDEO_MENU_FULLSCREEN		WM_USER + 1979
#define		VIDEO_MENU_MULTISCREEN		WM_USER + 1980
#define		VIDEO_MENU_AUTOADJUST		WM_USER + 1981
#define		VIDEO_MENU_RECORDVIDEO		WM_USER + 1982
#define		VIDEO_MENU_PRINTSCREEN		WM_USER + 1983
#define		VIDEO_MENU_EXITDECODE		WM_USER + 1984
#define		VIDEO_MENU_EXITCYCLE		WM_USER + 1985
#define		VIDEO_REPAINT				WM_USER + 1999


//	KeyColor

//#define		VIDEO_BACK_COLOR	RGB(111,104,160)
#define	VIDEO_BACK_COLOR				RGB(100,100,160)
// #define NAME_MENU_FULLSCREEN			"全屏显示"
// #define NAME_MENU_MULTISCREEN			"多屏显示"
// #define NAME_MENU_AUTOADJUST			"自动调整"
// #define NAME_MENU_EXITDECODE			"关闭解码"
// #define NAME_MENU_EXITCYCLE				"关闭轮巡"

/////////////////////////////////////////////////////////////////////////////
// CPlayWnd dialog

class CPlayWnd : public CWnd
{
	// Construction
public:
	CPlayWnd();   // standard constructor
	
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayWnd)
public:
	virtual BOOL DestroyWindow();
protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	afx_msg void OnVideoMenu(UINT nID);
	afx_msg LRESULT OnRepaintWnd(WPARAM wParam, LPARAM lParam);
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CPlayWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

#if _MSC_VER >= 1300
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#else
	afx_msg void OnTimer(UINT nIDEvent);
#endif

#if _MSC_VER >= 1300
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
#else
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#endif
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
public:
	void SetWinID(int ID) {m_nWndID = ID;}
	int  GetWinID(void){return m_nWndID;}
    void  SetWndPlaying(bool bPlay){ bIsPlaying = bPlay; }
	
private:
	int m_nWndID;
	CPoint pointStart;
	CPoint pointEnd;
	CPoint pointMove;
	BOOL  m_FlagRect;
    bool  bIsPlaying;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYWND_H__158FCA3F_D545_4DB6_9946_0FC9F7D9D5CE__INCLUDED_)
