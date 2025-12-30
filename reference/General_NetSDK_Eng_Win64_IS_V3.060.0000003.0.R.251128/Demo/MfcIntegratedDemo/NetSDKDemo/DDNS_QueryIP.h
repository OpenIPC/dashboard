#if !defined(AFX_DDNS_QUERYIP_H__2F22A260_B43D_49E9_8816_48D1534FFCCF__INCLUDED_)
#define AFX_DDNS_QUERYIP_H__2F22A260_B43D_49E9_8816_48D1534FFCCF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DDNS_QueryIP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDDNS_QueryIP dialog

class CDDNS_QueryIP : public CDialog
{
// Construction
public:
	CDDNS_QueryIP(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDDNS_QueryIP)
	enum { IDD = IDD_DDNS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDDNS_QueryIP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDDNS_QueryIP)
	afx_msg void OnGo();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DDNS_QUERYIP_H__2F22A260_B43D_49E9_8816_48D1534FFCCF__INCLUDED_)
