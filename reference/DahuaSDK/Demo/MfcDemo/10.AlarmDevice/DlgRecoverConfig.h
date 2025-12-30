#if !defined(AFX_DLGRECOVERCONFIG_H__3F973D71_C297_4782_89F5_76D4C19BA73C__INCLUDED_)
#define AFX_DLGRECOVERCONFIG_H__3F973D71_C297_4782_89F5_76D4C19BA73C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgRecoverConfig.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CDlgRecoverConfig dialog

class CDlgRecoverConfig : public CDialog
{
// Construction
public:
	CDlgRecoverConfig(CWnd* pParent = NULL, LLONG lLoginId = 0, NET_DEVICE_TYPE emType = NET_PRODUCT_NONE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgRecoverConfig)
	enum { IDD = IDD_DLG_RECOVERCONFIG };
	CComboBox	m_ctlReverType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRecoverConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgRecoverConfig)
	afx_msg void OnRecoverConfigBtnReset();
	virtual BOOL OnInitDialog();
	afx_msg void OnRecoverConfigBtnResetEx();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LLONG			m_lLoginId;
    NET_DEVICE_TYPE	m_emType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGRECOVERCONFIG_H__3F973D71_C297_4782_89F5_76D4C19BA73C__INCLUDED_)
