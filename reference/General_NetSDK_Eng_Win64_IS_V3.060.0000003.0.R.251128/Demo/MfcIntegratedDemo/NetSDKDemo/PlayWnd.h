#if !defined(AFX_PLAYWND_H__E6AAF690_8D19_43AC_AE4E_E64F2412D706__INCLUDED_)
#define AFX_PLAYWND_H__E6AAF690_8D19_43AC_AE4E_E64F2412D706__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlayWnd.h : header file
//

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
#define		VIDEO_BACK_COLOR	RGB(100,100,160)

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
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetWinID(int ID) {m_nWndID = ID;}
	int  GetWinID(void){return m_nWndID;}

	//get split info 
	BOOL			GetSplitInfo(SplitInfoNode* info);
	//set split info -- copy whole structure
	BOOL			SetSplitInfo(SplitInfoNode* info);
	//set split info -- split type
	void			SetSplitType(SplitType type);
	//set split info -- channel handle
	void			SetSplitHandle(LLONG dwHandle);
	//set split info -- split parameter
	void			SetSplitParam(void *Param);
	//set split info -- flages of call-back-data saving 
	void			SetSplitCBFlag_Real(int nFlag);
	void			SetSplitCBFile_Raw(FILE *file);
	void			SetSplitCBFile_Std(FILE *file);
	void			SetSplitCBFile_Pcm(FILE *file);
	void			SetSplitCBFile_Yuv(FILE *file);
	
private:
	CRITICAL_SECTION m_cs;
	BOOL m_bFullScreen;
	int m_nWndID;

	SplitInfoNode m_splitInfo;
	int m_nPreSplit;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYWND_H__E6AAF690_8D19_43AC_AE4E_E64F2412D706__INCLUDED_)
