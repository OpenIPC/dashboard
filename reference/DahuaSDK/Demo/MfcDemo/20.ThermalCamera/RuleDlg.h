#if !defined(AFX_RULEDLG_H__5FFE7B73_BB84_40E9_A39B_3EFD2F7861FC__INCLUDED_)
#define AFX_RULEDLG_H__5FFE7B73_BB84_40E9_A39B_3EFD2F7861FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RuleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRuleDlg dialog

class CRuleDlg : public CDialog
{
// Construction
public:
	CRuleDlg(CWnd* pParent = NULL,LLONG lLoginId = 0, int nChannel = 0);   // standard constructor
// Dialog Data
	//{{AFX_DATA(CRuleDlg)
	enum { IDD = IDD_DLG_RULE };
	CComboBox	m_Combo_AlarmID;
	CComboBox	m_Combo_LocalConfig;
	CComboBox	m_Combo_Ruleid;
	CComboBox	m_Combo_RuleNum;
	CComboBox	m_Combo_AlarmEnable;
	CComboBox	m_Combo_Condition;
	CComboBox	m_Combo_ResultType;
	CComboBox	m_Combo_PointNum;
	CComboBox	m_Combo_ModeType;
	CComboBox	m_ComboEnable;
	int		m_nRuleAmount;
	int		m_nPresetNum;
	int		m_nRule;
	CString	m_szCustomName;
	int		m_nCooddinatecount;
	int		m_nX;
	int		m_nY;
	int		m_nPeriod;
	int		m_nAlarmid;
	float	m_fHysteresis;
	float	m_fThreshold;
	int		m_nDrationTime;
	float	m_fObjectEmissivity;
	int		m_nObjectDistance;
	int		m_nRefalectedTemp;
	int		m_nAlarmAmount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRuleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_iLoginID;
    int m_nChannel;
    BOOL GetRuleConfig(int RuleID,int pointID,int AlarmID);
    BOOL SetRuleConfig();
	// Generated message map functions
	//{{AFX_MSG(CRuleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnGet();
	afx_msg void OnBtnSet();
	afx_msg void OnSelchangeComboRuleid();
	afx_msg void OnSelchangeComboPointnum();
	afx_msg void OnSelchangeComboAlarmid();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RULEDLG_H__5FFE7B73_BB84_40E9_A39B_3EFD2F7861FC__INCLUDED_)
