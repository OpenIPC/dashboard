#if !defined(AFX_USER_ADDUSER_H__CBD6B13E_328E_420B_8664_564102DA8C9D__INCLUDED_)
#define AFX_USER_ADDUSER_H__CBD6B13E_328E_420B_8664_564102DA8C9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// User_AddUser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUser_AddUser dialog

class CUser_AddUser : public CDialog
{
// Construction
public:
	CUser_AddUser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUser_AddUser)
	enum { IDD = IDD_USER_ADD_USER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUser_AddUser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUser_AddUser)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOk();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD nIndex, DeviceNode *dev);
	void RefreshRightList(DWORD *dwList);
private:
	USER_MANAGE_INFO_NEW *m_user_info;
	DeviceNode *m_dev;
	DWORD m_nGroupIndex;  // 用户组下表
	CTreeCtrl m_rightList;  
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USER_ADDUSER_H__CBD6B13E_328E_420B_8664_564102DA8C9D__INCLUDED_)
