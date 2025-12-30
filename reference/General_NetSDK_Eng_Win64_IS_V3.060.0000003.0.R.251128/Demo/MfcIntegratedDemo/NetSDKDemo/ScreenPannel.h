#if !defined(AFX_SCREENPANNEL_H__F79F0D7D_1068_4E35_8380_22AE3E322707__INCLUDED_)
#define AFX_SCREENPANNEL_H__F79F0D7D_1068_4E35_8380_22AE3E322707__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScreenPannel.h : header file
#include "BSWndContainer.h"
#include "PlayWnd.h"
//

/////////////////////////////////////////////////////////////////////////////
// CScreenPannel class

class CScreenPannel : public CBSWndContainer
{
// Construction
public:
	CScreenPannel();


// Implementation
public:
	virtual ~CScreenPannel();

	// Generated message map functions
protected:
	//{{AFX_MSG(CScreenPannel)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//现在nSub没有用,请填零
	int SetShowPlayWin(int nMain, int nSub=0);
	CWnd * GetPage(int nIndex);

////////////////////////////////////////////////////////////////////////////////////////////////////////
// private member for inter user
private:
	
	CPlayWnd m_wndVideo[DH_MAX_CHANNUM];
	int nLastSplit;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCREENPANNEL_H__F79F0D7D_1068_4E35_8380_22AE3E322707__INCLUDED_)
