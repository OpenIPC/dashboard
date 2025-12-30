#if !defined(AFX_DLGCFGLOCALES_H__211413D5_87ED_4330_BB98_3A9D528896D3__INCLUDED_)
#define AFX_DLGCFGLOCALES_H__211413D5_87ED_4330_BB98_3A9D528896D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgLocales.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgLocales dialog

class CDlgCfgLocales : public CDialog
{
// Construction
public:
	CDlgCfgLocales(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgLocales)
	enum { IDD = IDD_DLG_CFG_LOCALES };
	CButton	m_chkDST;
	CComboBox	m_cbWeekEnd;
	CComboBox	m_cbWeekBegin;
	CComboBox	m_cbMonthEnd;
	CComboBox	m_cbMonthBegin;
	CComboBox	m_cbMinEnd;
	CComboBox	m_cbMinBegin;
	CComboBox	m_cbHourEnd;
	CComboBox	m_cbHourBegin;
	CDateTimeCtrl	m_dtpDateEnd;
	CDateTimeCtrl	m_dtpDateBegin;
	CComboBox	m_cbWeekdayBegin;
	CComboBox	m_cbWeekdayEnd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgLocales)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CDlgCfgLocales)
	virtual BOOL OnInitDialog();
	afx_msg void OnLocalesRadioByday();
	afx_msg void OnLocalesRadioByweekday();
	afx_msg void OnLocalesBtnGet();
	afx_msg void OnLocalesBtnSet();
	afx_msg void OnLocalesChkDst();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void	InitDlg(BOOL bRecheck = TRUE);
	BOOL	GetConfig();
	BOOL	SetConfig();
	void	StuToDlg();
	void	DlgToStu();
	
private:
	LLONG			m_lLoginId;
	AV_CFG_Locales	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGLOCALES_H__211413D5_87ED_4330_BB98_3A9D528896D3__INCLUDED_)
