#if !defined(AFX_USERMANAGE_H__AB653496_122E_4896_B716_1A9AF6382367__INCLUDED_)
#define AFX_USERMANAGE_H__AB653496_122E_4896_B716_1A9AF6382367__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserManage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserManage dialog

class CUserManage : public CDialog
{
// Construction
public:
	CUserManage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUserManage)
	enum { IDD = IDD_USER_MANAGE };

	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserManage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserManage)
	virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
	afx_msg void OnAddUser();
	afx_msg void OnModifyUser();
	afx_msg void OnAddGroup();
	afx_msg void OnDeleteUser();
	afx_msg void OnModifyPsw();
	afx_msg void OnDeleteGroup();
	afx_msg void OnModifyGroup();
	afx_msg void OnSelchangedUserlistTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangedRightlistTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
    // 设置设备登陆信息
	void SetDeviceId(DeviceNode *dev);

    // 刷新权限列表
	void RefreshRightList(DWORD *dwList);

    // 刷新用户列表和权限列表
	void RefreshUserAndRight();

    // 根据组ID获取组下表值
    int GetGroupIndex(int nGroupId);

    // 初始化用户信息结构体
    void InitStuUserInfo(USER_MANAGE_INFO_NEW* pInfo);

    // 判断当前节点是否为父节点
    bool isParentNode(HTREEITEM node);

private:
	DeviceNode              *m_dev;       // 设备登陆信息
	USER_MANAGE_INFO_NEW*   m_pUserInfo;  // 用户信息
	CTreeCtrl               m_rightList;  // 权限列表
    CTreeCtrl	            m_userList;   // 用户组和用户名
public:
   
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERMANAGE_H__AB653496_122E_4896_B716_1A9AF6382367__INCLUDED_)
