#if !defined(AFX_CDlgCfgAirCondition_H__C3869A9D_5797_441F_84DD_975E1ACC572C__INCLUDED_)
#define AFX_CDlgCfgAirCondition_H__C3869A9D_5797_441F_84DD_975E1ACC572C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAirCondition.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAirCondition dialog

class CDlgCfgAirCondition : public CDialog
{
// Construction
public:
	CDlgCfgAirCondition(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAirCondition)
	enum { IDD = IDD_DLG_CFG_AIRCONDITION };
    CComboBox	m_cmbDevID;
	CString	    m_strName;
    CString     m_strBrand;
    CButton	    m_ctlON;
    UINT        m_nRange;
    CString     m_strAddr;
    CComboBox	m_cmbMode;
    CComboBox	m_cmbWindMode;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAirCondition)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAirCondition)
    virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeCfgCmbDeviceid();
	afx_msg void OnCfgAirconditionBtnGet();
	afx_msg void OnCfgAirconditionBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void		InitDlg();
    void		StuToDlg();
    void		DlgToStu();
    BOOL		GetConfig();
    BOOL		SetConfig();
    
private:
    LLONG						m_lLoginID;
    int                         m_nCurrentDevID;
	CFG_AIRCONDITION_INFO	    m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDlgCfgAirCondition_H__C3869A9D_5797_441F_84DD_975E1ACC572C__INCLUDED_)
