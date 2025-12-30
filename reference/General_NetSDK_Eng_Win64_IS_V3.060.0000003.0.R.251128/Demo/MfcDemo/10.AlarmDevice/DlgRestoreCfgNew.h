#if !defined(AFX_DLGRESTORECFGNEW_H__1B08327B_53FA_4FF0_95F4_604D16A912F0__INCLUDED_)
#define AFX_DLGRESTORECFGNEW_H__1B08327B_53FA_4FF0_95F4_604D16A912F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgRestoreCfgNew.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgRestoreCfgNew dialog

class CDlgRestoreCfgNew : public CDialog
{
// Construction
public:
	CDlgRestoreCfgNew(CWnd* pParent /* = NULL */, LLONG hLoginId /* = NULL */);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgRestoreCfgNew)
	enum { IDD = IDD_DLG_RECOVERCONFIG_EX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRestoreCfgNew)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

    // Implementation
 protected:
    // Generated message map functions
    //{{AFX_MSG(CDlgRestoreCfg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
            
 private:
	void	InitDlg();

 private:
    LLONG       m_hLoginId;
    CButton*	m_pbtnCheck;

    typedef enum emBtnID
    {
        emBtnID_base = 0xfa0,
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGRESTORECFGNEW_H__1B08327B_53FA_4FF0_95F4_604D16A912F0__INCLUDED_)
