#if !defined(AFX_QUERYPOINTINFODLG_H__764507F6_ADE5_48EA_BAC9_AC3F7EB20167__INCLUDED_)
#define AFX_QUERYPOINTINFODLG_H__764507F6_ADE5_48EA_BAC9_AC3F7EB20167__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueryPointInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQueryPointInfoDlg dialog

class CQueryPointInfoDlg : public CDialog
{
// Construction
public:
	CQueryPointInfoDlg(CWnd* pParent = NULL,LLONG iLoginId = 0, int nChannel = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQueryPointInfoDlg)
	enum { IDD = IDD_DLG_POINTINFO };
	int		m_nX; // [0, 8191]
	int		m_nY; // [0, 8191]
	CString	m_type;
	CString	m_unit;
	float	m_temp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueryPointInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_lLoginID;
    int m_nChannel;
	// Generated message map functions
	//{{AFX_MSG(CQueryPointInfoDlg)
	afx_msg void OnPointquery();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUERYPOINTINFODLG_H__764507F6_ADE5_48EA_BAC9_AC3F7EB20167__INCLUDED_)
