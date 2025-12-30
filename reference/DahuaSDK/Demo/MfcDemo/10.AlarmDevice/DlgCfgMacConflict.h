#if !defined(AFX_DLGCFGMACCONFLICT_H__762C3BA6_E55D_4DE8_8505_C5B1E95CC068__INCLUDED_)
#define AFX_DLGCFGMACCONFLICT_H__762C3BA6_E55D_4DE8_8505_C5B1E95CC068__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgMacConflict.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMacConflict dialog

class CDlgCfgMacConflict : public CDialog
{
// Construction
public:
	CDlgCfgMacConflict(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgMacConflict)
	enum { IDD = IDD_DLG_CFG_MACCONFLICT };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgMacConflict)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgMacConflict)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgmacconflictBtnEventhandler();
	afx_msg void OnCfgmacconflictBtnGet();
	afx_msg void OnCfgmacconflictBtnSet();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
        
private:
    BOOL getInfo();
    BOOL showInfo();
    BOOL SetConfigToDevice();
    BOOL GetConfigFromDevice();
    
private:
    LLONG               m_lLoginId;
	CFG_MACCONFLICT_INFO m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGMACCONFLICT_H__762C3BA6_E55D_4DE8_8505_C5B1E95CC068__INCLUDED_)
