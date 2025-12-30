#if !defined(AFX_DLGUPGRADE_H__77CB86C2_60C6_4052_89B0_99BFD68E9E6B__INCLUDED_)
#define AFX_DLGUPGRADE_H__77CB86C2_60C6_4052_89B0_99BFD68E9E6B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgUpgrade.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgUpgrade dialog

class CDlgUpgrade : public CDialog
{
// Construction
public:
	CDlgUpgrade(CWnd* pParent = NULL, LLONG hLoginId = NULL);   // standard constructor
	
public:
	friend void WINAPI UpgradeCallBack(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize, LDWORD dwUser);
	void UpgradeState(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize);

// Dialog Data
	//{{AFX_DATA(CDlgUpgrade)
	enum { IDD = IDD_DLG_UPGRADE };
	CListBox	m_lsInfo;
	CProgressCtrl	m_ctrlProgress;
	CStatic	m_staticProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUpgrade)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgUpgrade)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnUpgradeBtnPath();
	afx_msg void OnUpgradeBtnUpgrade();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	StopUpgrade(LLONG lUpgradId = NULL);

private:
	LLONG	m_hLoginId;
	LLONG	m_hUpgradeId;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGUPGRADE_H__77CB86C2_60C6_4052_89B0_99BFD68E9E6B__INCLUDED_)
