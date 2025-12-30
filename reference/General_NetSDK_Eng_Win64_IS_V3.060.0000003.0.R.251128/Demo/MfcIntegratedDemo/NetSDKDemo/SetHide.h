#if !defined(AFX_SETHIDE_H__4AB9B2C9_CB22_43F9_ABE1_225DA9792FD4__INCLUDED_)
#define AFX_SETHIDE_H__4AB9B2C9_CB22_43F9_ABE1_225DA9792FD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetHide.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetHide dialog

class CSetHide : public CDialog
{
// Construction
public:
	CSetHide(CWnd* pParent = NULL);   // standard constructor

	SetShelter(int nIndex, NET_DEV_SHELTER *shelter);
	GetShelter(int nIndex, NET_DEV_SHELTER *shelter);

// Dialog Data
	//{{AFX_DATA(CSetHide)
	enum { IDD = IDD_SETHIDE };
	CStatic	m_DetectPic;
	BOOL	m_bSetHide;
	BOOL	m_bShowHide;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetHide)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetHide)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowhideCheck();
	afx_msg void OnSethideCheck();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSetHide();
	afx_msg void OnBtnquit();
	afx_msg void OnSetHideOK();
	afx_msg void OnQuit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	NET_DEV_SHELTER m_struShelter[MAX_SHELTERNUM];

	BOOL m_bDrawdetect; //标记是否正在设置区域

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETHIDE_H__4AB9B2C9_CB22_43F9_ABE1_225DA9792FD4__INCLUDED_)
