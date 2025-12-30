#if !defined(AFX_DLGCFGIPCONFLICT_H__0CE488FE_15AD_41DF_8B11_BFD144607F14__INCLUDED_)
#define AFX_DLGCFGIPCONFLICT_H__0CE488FE_15AD_41DF_8B11_BFD144607F14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgIPConflict.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgIPConflict dialog

class CDlgCfgIPConflict : public CDialog
{
// Construction
public:
	CDlgCfgIPConflict(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgIPConflict)
	enum { IDD = IDD_DLG_CFG_IPCONFLICT };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgIPConflict)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgIPConflict)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgipconflictBtnEventhandler();
	afx_msg void OnCfgipconflictBtnGet();
	afx_msg void OnCfgipconflictBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
        
private:
    BOOL getInfo();
    BOOL showInfo();
    BOOL SetConfigToDevice();
    BOOL GetConfigFromDevice();
    
private:
    LLONG               m_lLoginId;
	CFG_IPCONFLICT_INFO m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGIPCONFLICT_H__0CE488FE_15AD_41DF_8B11_BFD144607F14__INCLUDED_)
