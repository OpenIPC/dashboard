#if !defined(AFX_EXCEPTIONCONFIG_H__4B61916C_76A4_4B7C_981F_F36785A9077E__INCLUDED_)
#define AFX_EXCEPTIONCONFIG_H__4B61916C_76A4_4B7C_981F_F36785A9077E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExceptionConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExceptionConfig dialog

class CExceptionConfig : public CDialog
{
// Construction
public:
	CExceptionConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExceptionConfig)
	enum { IDD = IDD_CHILD_EXCEPTIONCFG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExceptionConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExceptionConfig)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXCEPTIONCONFIG_H__4B61916C_76A4_4B7C_981F_F36785A9077E__INCLUDED_)
