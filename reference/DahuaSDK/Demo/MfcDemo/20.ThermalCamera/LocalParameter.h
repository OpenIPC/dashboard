#if !defined(AFX_LOCALPARAMETER_H__C38C5BDE_405A_44B5_9EF2_BC5FC776533A__INCLUDED_)
#define AFX_LOCALPARAMETER_H__C38C5BDE_405A_44B5_9EF2_BC5FC776533A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocalParameter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLocalParameter dialog

class CLocalParameter : public CDialog
{
// Construction
public:
	CLocalParameter(CWnd* pParent = NULL,LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLocalParameter)
	enum { IDD = IDD_DLG_LOCAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocalParameter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_iLoginID;
	// Generated message map functions
	//{{AFX_MSG(CLocalParameter)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCALPARAMETER_H__C38C5BDE_405A_44B5_9EF2_BC5FC776533A__INCLUDED_)
