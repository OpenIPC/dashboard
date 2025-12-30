#if !defined(AFX_TABUSERMANAGEDLG_H__9B356D20_D1A2_47DA_99AD_DBCD2805993E__INCLUDED_)
#define AFX_TABUSERMANAGEDLG_H__9B356D20_D1A2_47DA_99AD_DBCD2805993E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabUserManageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabUserManageDlg dialog

class CTabUserManageDlg : public CDialog
{
	// Construction
public:	
	CTabUserManageDlg(CWnd* pParent = NULL, LLONG nLoginId = 0, NET_DEVICE_TYPE emDevType = NET_PRODUCT_NONE);   // standard constructor
    ~CTabUserManageDlg();

// Dialog Data
	//{{AFX_DATA(CTabUserManageDlg)
	enum { IDD = IDD_DIALOG_USERMANAGE };
	CTreeCtrl	m_treeLevels;
	CTreeCtrl	m_treeUsers;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabUserManageDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CTabUserManageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTreeUsers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddUser();
	afx_msg void OnModifyUser();
	afx_msg void OnDeleteUser();
	afx_msg void OnModifyPsw();
	afx_msg void OnAddGroup();
	afx_msg void OnDeleteGroup();
	afx_msg void OnModifyGroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LLONG					m_hLoginID;
	NET_DEVICE_TYPE			m_emDevType;
	USER_MANAGE_INFO_NEW*	m_pstuUsersInfo;

private:
	void					RefreshLevelList();
	void					InitTree();
	void					CleanUserManageInfo();
    void                    InitUserInfo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABUSERMANAGEDLG_H__9B356D20_D1A2_47DA_99AD_DBCD2805993E__INCLUDED_)
