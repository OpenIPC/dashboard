#if !defined(AFX_DLGSETARMMODE_H__4E47C17B_86D1_42F1_BEE8_CD5FBC6737C8__INCLUDED_)
#define AFX_DLGSETARMMODE_H__4E47C17B_86D1_42F1_BEE8_CD5FBC6737C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSetArmMode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSetArmMode dialog

class CDlgSetArmMode : public CDialog
{
// Construction
public:
	CDlgSetArmMode(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSetArmMode)
	enum { IDD = IDD_DLG_SETARMMODE };
	CComboBox	m_cmbScene;
	CComboBox	m_cmbMode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetArmMode)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSetArmMode)
	afx_msg void OnSetarmmodeBtnSet();
	virtual BOOL OnInitDialog();
	afx_msg void OnSetarmmodeBtnAlarmSource();
	afx_msg void OnSetarmmodeBtnAlarmLink();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    LLONG                           m_lLoginId;
    CTRL_ARM_DISARM_PARAM_EX_OUT    m_stuOut;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETARMMODE_H__4E47C17B_86D1_42F1_BEE8_CD5FBC6737C8__INCLUDED_)
