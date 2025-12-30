#if !defined(AFX_EXBUTTON_H__7F81B928_9773_46C2_BCC3_04D729A7A614__INCLUDED_)
#define AFX_EXBUTTON_H__7F81B928_9773_46C2_BCC3_04D729A7A614__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExButton window

class CExButton : public CButton
{
// Construction
public:
	CExButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	//Set PTZ control command 
	void SetButtonCommand(DWORD dwPTZCommand);
	virtual ~CExButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CExButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	DWORD m_dwPTZCommand;
	BOOL m_bIsMouseDown;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXBUTTON_H__7F81B928_9773_46C2_BCC3_04D729A7A614__INCLUDED_)
