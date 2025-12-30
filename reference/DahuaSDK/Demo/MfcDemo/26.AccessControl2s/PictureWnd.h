#if !defined(AFX_PICTUREWND_H__900C9FD1_3B99_4CE5_9FB7_8BDBE1AE42E6__INCLUDED_)
#define AFX_PICTUREWND_H__900C9FD1_3B99_4CE5_9FB7_8BDBE1AE42E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PictureWnd.h : header file
//

#include <afxmt.h>
#define	PICTURE_BACK_COLOR	RGB(105,105,105)

/////////////////////////////////////////////////////////////////////////////
// CPictureWnd window

class CPictureWnd : public CWnd
{
// Construction
public:
	CPictureWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPictureWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPictureWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	IPicture*			m_pIPicture;
	CCriticalSection	m_csIPictrue;
	HGLOBAL				m_hImageMemory;
	SCROLLINFO			m_siVert;
	SCROLLINFO			m_siHorz;
	int					m_nScale;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTUREWND_H__900C9FD1_3B99_4CE5_9FB7_8BDBE1AE42E6__INCLUDED_)
