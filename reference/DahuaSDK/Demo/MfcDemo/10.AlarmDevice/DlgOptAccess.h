#if !defined(AFX_DLGOPTACCESS_H__B28733E3_62E9_42B0_802A_3D641A8D51CF__INCLUDED_)
#define AFX_DLGOPTACCESS_H__B28733E3_62E9_42B0_802A_3D641A8D51CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOptAccess.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgOptAccess dialog

class CDlgOptAccess : public CDialog
{
// Construction
public:
	CDlgOptAccess(CWnd* pParent = NULL, LLONG hLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgOptAccess)
	enum { IDD = IDD_DLG_OPT_ACCESS };
	CComboBox	m_cbChannelId;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgOptAccess)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgOptAccess)
	afx_msg void OnBtnCancel();
	afx_msg void OnBtnOpen();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG m_hLoginID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPTACCESS_H__B28733E3_62E9_42B0_802A_3D641A8D51CF__INCLUDED_)
