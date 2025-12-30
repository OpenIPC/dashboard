#if !defined(AFX_DLGCFGGUARD_H__F9841EBC_83E4_4147_8994_1BE14B3F522F__INCLUDED_)
#define AFX_DLGCFGGUARD_H__F9841EBC_83E4_4147_8994_1BE14B3F522F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgGuard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgGuard dialog

class CDlgCfgGuard : public CDialog
{
// Construction
public:
	CDlgCfgGuard(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor
	BOOL GetCurrentGuardState();
	void StuToDlg();
	void DlgToStu(int nIndex = 0);

// Dialog Data
	//{{AFX_DATA(CDlgCfgGuard)
	enum { IDD = IDD_DLG_CFG_GUARD };
	CComboBox	m_cmbScene;
	CEdit	m_edRetSceneCount;
	CComboBox	m_cmbSceneName;
	CComboBox	m_cmbCurrentScene;
	CButton	m_ckAlarmEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgGuard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgGuard)
	virtual BOOL OnInitDialog();
	afx_msg void OnGuardBtnGet();
	afx_msg void OnGuardBtnSet();
	afx_msg void OnSelchangeGuardCmbScene();
	afx_msg void OnDropdownGuardCmbScene();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG					m_lLoginID;
	CFG_COMMGLOBAL_INFO		m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGGUARD_H__F9841EBC_83E4_4147_8994_1BE14B3F522F__INCLUDED_)
