#if !defined(AFX_VIDEOSCREEN_H__75053308_8DC0_4DC3_9DCF_160F603B5108__INCLUDED_)
#define AFX_VIDEOSCREEN_H__75053308_8DC0_4DC3_9DCF_160F603B5108__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoScreen.h : header file
//
#include <vector>
class CVideoWnd;

#define WM_CLOSE_VIDEOWINDOW		WM_USER + 0X0230
#define WM_SET_VIDEOWINDOW_RECT		WM_USER + 0X0231
#define WM_CLEAR_VIDEOWINDOW		WM_USER + 0X0232

/////////////////////////////////////////////////////////////////////////////
// CVideoScreen window

class CVideoScreen : public CWnd
{
// Construction
public:
	CVideoScreen();

// Attributes
public:
	enum WindowControlType
	{
		FIXED_WINDOW,	// 定窗模式, 1, 4, 9, 16分割...
		SLIP_WINDOW		// 滑窗模式, 自由开窗
	};

	enum { NONE, MOVE, ZOOM };

// Operations
public:
	void SetWindowControlType(WindowControlType emType);
	WindowControlType GetWindowControlType() const { return m_emWndCtrlType; }

	void SetSplitMode(UINT nSplit);
	UINT GetSplitMode() const { return m_vecWnd.size(); }
	void SetActiveWindow(const CVideoWnd* pWnd);
	void SetActiveWindow(int nWndID);
	CVideoWnd* GetActiveWindow() const { return m_vecWnd[m_nCurSel]; }
	int	 GetActiveWindowID() const { return m_nCurSel; }
	
	void SetWindowText(UINT nWndID, LPCTSTR lpszText);
	CString GetWindowText(UINT nWndId);
	void ClearWindowText();

	BOOL AddWindow(const CRect& rect, UINT nWndID, LPCTSTR lpszText = NULL);
	void RemoveWindow(UINT nWndID);
	
	CVideoWnd* GetWindow(UINT nWndID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoScreen)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

	virtual void OnVideoLButtonDown(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoLButtonUp(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoLButtonDblClk(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoRButtonDown(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoRButtonUp(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoMove(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoMoving(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnVideoMouseMove(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnCloseButtonClicked(int nWndID, WPARAM wParam, LPARAM lParam);
	virtual void OnClearButtonClicked(int nWndID, WPARAM wParam, LPARAM lParam);

// Implementation
public:
	virtual ~CVideoScreen();

protected:
	void Clear();
	void Dispose();
	void DrawActiveWindow(BOOL bActive = TRUE);

protected:
	std::vector<CVideoWnd*>	m_vecWnd;
	int					m_nCurSel;
	WindowControlType	m_emWndCtrlType;
	CPoint				m_ptStart;
	CRect				m_rcStart;
	UINT				m_nAction;

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoScreen)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOSCREEN_H__75053308_8DC0_4DC3_9DCF_160F603B5108__INCLUDED_)
