#pragma once
#include "afxcmn.h"
#include "UserInfo.h"

// UserManage 对话框

class UserManage : public CDialog
{
	DECLARE_DYNAMIC(UserManage)

public:
	UserManage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~UserManage();

// 对话框数据
	enum { IDD = IDD_DIALOG_USERMANAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void ListReload();
	std::vector<NET_ACCESS_USER_INFO> m_UserInfoVector;
	int m_nUserInfoIndex;
public:
	CListCtrl m_list_user;
	afx_msg void OnBnClickedButUserAdd();
	afx_msg void OnBnClickedButUserModify();
	afx_msg void OnBnClickedButUserDelete();
	afx_msg void OnBnClickedButUserGet();
	void UIState(BOOL b);
	void DoFindNextUserRecord();

	LRESULT ListLoad(WPARAM wParam, LPARAM lParam);
	LRESULT CtrlEnable(WPARAM wParam, LPARAM lParam);
	HANDLE  m_ThreadHandle;

	BOOL	m_bIsDoFindNext;
	DWORD   m_dwThreadID;
	afx_msg void OnNMClickListctrlUser(NMHDR *pNMHDR, LRESULT *pResult);

	HWND			hParentWnd;
	UserInfo*     pUserInfoDlg;
};
