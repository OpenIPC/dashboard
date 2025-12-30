#if !defined(AFX_DLGVERSION_H__D0D0FF34_6DD5_4803_B287_BE3BDC6E1BA0__INCLUDED_)
#define AFX_DLGVERSION_H__D0D0FF34_6DD5_4803_B287_BE3BDC6E1BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgVersion.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgVersion dialog

class CDlgVersion : public CDialog
{
// Construction
public:
	CDlgVersion(CWnd* pParent = NULL, LLONG hLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgVersion)
	enum { IDD = IDD_DLG_VERSION };
	CStatic	m_staVersion;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgVersion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgVersion)
	virtual BOOL OnInitDialog();
	afx_msg void OnVersionUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG	m_hLoginId;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGVERSION_H__D0D0FF34_6DD5_4803_B287_BE3BDC6E1BA0__INCLUDED_)
