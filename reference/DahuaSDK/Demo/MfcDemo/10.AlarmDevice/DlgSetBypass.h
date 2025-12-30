#if !defined(AFX_DLGSETBYPASS_H__B136E901_EDEE_4960_A816_DBE012242014__INCLUDED_)
#define AFX_DLGSETBYPASS_H__B136E901_EDEE_4960_A816_DBE012242014__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSetBypass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSetBypass dialog

class CDlgSetBypass : public CDialog
{
// Construction
public:
	CDlgSetBypass(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSetBypass)
	enum { IDD = IDD_DLG_SET_BYPASS };
	CEdit	m_edPassword;
	CButton	m_ckMode;
	UINT	m_edExtendedCount;
	UINT	m_edLocalCount;
	UINT	m_edExtendedCountForGet;
	UINT	m_edLocalCountForGet;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetBypass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSetBypass)
	afx_msg void OnSetbypassButtonLocal();
	afx_msg void OnSetbypassButtonExtended();
	afx_msg void OnDestroy();
	afx_msg void OnSetbypassButtonSet();
	afx_msg void OnSetbypassButtonExtendedForget();
	afx_msg void OnSetbypassButtonGet();
	afx_msg void OnSetbypassButtonLocalForget();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG					m_lLoginId;
	NET_CTRL_SET_BYPASS		m_stuSetBypass;
	NET_DEVSTATE_GET_BYPASS m_stuGetBypass;
	BOOL					m_bNeedMode;
	int						m_nLastLocalCount;
	int						m_nLastExtendedCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETBYPASS_H__B136E901_EDEE_4960_A816_DBE012242014__INCLUDED_)
