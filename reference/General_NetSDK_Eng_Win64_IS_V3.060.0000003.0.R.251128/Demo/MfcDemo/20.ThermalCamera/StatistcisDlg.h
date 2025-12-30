#if !defined(AFX_STATISTCISDLG_H__42E6437B_3114_44B4_8C07_300AA55F2F9B__INCLUDED_)
#define AFX_STATISTCISDLG_H__42E6437B_3114_44B4_8C07_300AA55F2F9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatistcisDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatistcisDlg dialog

class CStatistcisDlg : public CDialog
{
// Construction
public:
	CStatistcisDlg(CWnd* pParent = NULL,LLONG lLoginId = 0);   // standard constructor
    
// Dialog Data
	//{{AFX_DATA(CStatistcisDlg)
	enum { IDD = IDD_DLG_STATISTCIS };
	CComboBox	m_Combo_TempStaNum;
	CComboBox	m_Combo_Temp;
	CComboBox	m_Combo_ModeType;
	int		m_Cycle;
	CString	m_ITemName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatistcisDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_iLoginID;
    CFG_TEMP_STATISTICS_INFO stuInfo;
	// Generated message map functions
	//{{AFX_MSG(CStatistcisDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboTempstanum();
	afx_msg void OnBtnGet();
	afx_msg void OnBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATISTCISDLG_H__42E6437B_3114_44B4_8C07_300AA55F2F9B__INCLUDED_)
