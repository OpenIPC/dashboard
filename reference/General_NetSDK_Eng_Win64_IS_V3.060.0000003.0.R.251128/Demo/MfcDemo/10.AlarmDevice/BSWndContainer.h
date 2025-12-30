/*********************************************************************************
 *	Name :	CBSWndContainer
 *
 *	Function :	When play multiple-channel video in one window, it is to control multiple-window.
 *			
 *			[one/multiple-window switch],[Full-screen],[Frame],[Window zoom in proportion],[Auto adjust width and height proportion]
 *		
 *	Writer :	OLinS
 *
 *	Time  :	2003.1.15
 *												
 *********************************************************************************
 */


#if !defined(AFX_BSWNDCONTAINER_H__73CB8E46_8ED9_4C36_BA91_29D5F5BB05DE__INCLUDED_)
#define AFX_BSWNDCONTAINER_H__73CB8E46_8ED9_4C36_BA91_29D5F5BB05DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BSWndContainer.h : header file
//
//Video window switch 
#define WINDOW_SPACE  1

/////////////////////////////////////////////////////////////////////////////
// CBSWndContainer window
#include <afxtempl.h>

class CBSWndContainer : public CWnd
{
// Construction
public:
	CBSWndContainer();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBSWndContainer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBSWndContainer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBSWndContainer)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////////////////
// public interface member
public:

	///////////////////////////////////////////////////
	// Create object (Container)
	BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	///////////////////////////////////////////////////
	// Add window 
	BOOL AddPage(CWnd *pWnd, BOOL bRepaint = TRUE);

	///////////////////////////////////////////////////
	// Delete window(Delete from the list,the real object need to be deleted from the external side.) 
	CWnd *DelPage(CWnd *pWnd);

	///////////////////////////////////////////////////
	// Delete activated window 
	CWnd *DelPage();

	///////////////////////////////////////////////////
	// Set activated window 
	void SetActivePage(CWnd *pWnd, BOOL bRepaint = TRUE);

	///////////////////////////////////////////////////
	// Get activated window 
	CWnd *GetActivePage();

	///////////////////////////////////////////////////
	// Get the next window 
	CWnd *GetNextPage(CWnd *pWnd);

	///////////////////////////////////////////////////
	// Get previous window 
	CWnd *GetPrevPage(CWnd *pWnd);

	///////////////////////////////////////////////////
	//	Get the specifed window 
	CWnd *GetPage(int nIndex);

	///////////////////////////////////////////////////
	//	Get window amount 
	int GetCount() const;

	///////////////////////////////////////////////////
	//	Get last window 
	CWnd *GetTailPage();

	///////////////////////////////////////////////////
	// Switch window 
	virtual void UpdateWnd();

	///////////////////////////////////////////////////
	//Full-screen display 
	void SetFullScreen(BOOL bFlag);
	BOOL GetFullScreen();

	///////////////////////////////////////////////////
	//Multiple-window display 
	void SetMultiScreen(BOOL bFlag);
	BOOL GetMultiScreen();

	//////////////////////////////////////////////////
	//Auto adjust dimension 
	void SetAutoAdjustPos(BOOL bFlag);
	BOOL GetAutoAdjustPos();

	//////////////////////////////////////////////////
	//	Activate window frame 
	void SetDrawActivePage(BOOL bFlag,COLORREF clrTopLeft=RGB(255, 0, 0), COLORREF clrBottomRight=RGB(255, 0, 0));
	BOOL GetDrawActivePage();

	//////////////////////////////////////////////////
	//	Display percentage
	//	40 <= nPortion <=100
	void SetShowPortion(int nPortion=100);
	int  GetShowPortion();

//////////////////////////////////////////////////////////////////////////////////////////////////////
// protected member for derived class
protected:
	
	///////////////////////////////////////////////////
	// Window list 
	CList<CWnd *,CWnd *> m_PageList;

	///////////////////////////////////////////////////
	// Activate window cursor 
	CWnd *m_pActivePage;

	///////////////////////////////////////////////////
	// Full screen sign 
	BOOL m_bFullScreen;

	///////////////////////////////////////////////////
	// Multiple-window sign 
	BOOL m_bMultiScreen;	

	///////////////////////////////////////////////////
	// Auto adjust sign 
	BOOL m_bAutoAdjustPos;

	///////////////////////////////////////////////////
	// Frame sign 
	BOOL m_bDrawActive;

	///////////////////////////////////////////////////
	// Display percentage(40-100)
	int	m_nShowPortion; 

////////////////////////////////////////////////////////////////////////////////////////////////////////
// private member for inter user
private:

	///////////////////////////////////////////////////
	//	Update list. Remove the invalid nodes among it and then return node amount.
	int UpdateList();

	///////////////////////////////////////////////////
	//	Get window postion in according to the small window serial number and displayed position
	virtual void CalcPageRect(LPRECT lpRect,int nIndex,int nPageCount);

	///////////////////////////////////////////////////
	//	Get displayed zone in pre-defined proportion (11/8)
	void AdjustRect(LPRECT lpRect);

	///////////////////////////////////////////////////
	//	Get displayed zone in proportion 
	void GetShowRect(LPRECT lpRect);

	///////////////////////////////////////////////////
	//	Activate window frame 
	void DrawActivePage(BOOL bFlag);

	///////////////////////////////////////////////////
	//	Window frame color 
	COLORREF m_clrTopLeft;
	COLORREF m_clrBottomRight;

	///////////////////////////////////////////////////
	//	Save original window message when in full screen to restore window.
	WINDOWPLACEMENT _temppl;		//window's placement
	CWnd *			_tempparent;	//window's parent
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BSWNDCONTAINER_H__73CB8E46_8ED9_4C36_BA91_29D5F5BB05DE__INCLUDED_)
