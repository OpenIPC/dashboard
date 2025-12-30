#if !defined(AFX_DLGUSERINFO_H__4684D34C_D99F_49A9_924E_18F66A550EF8__INCLUDED_)
#define AFX_DLGUSERINFO_H__4684D34C_D99F_49A9_924E_18F66A550EF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgUserInfo.h : header file
//

// 每次查询的最大用户个数
const int g_nCount = 10;

enum em_UserInfoType
{
	emUserInfoType_All = 0,
	emUserInfoType_Active,
};

typedef struct
{
	em_UserInfoType emTest;
	char*			pszName;
}Demo_UserInfoType;

const Demo_UserInfoType stuDemoUserInfoType[] = 
{
	{emUserInfoType_All, "All user"},
	{emUserInfoType_Active, "Active user"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgUserInfo dialog

class CDlgUserInfo : public CDialog
{
// Construction
public:
	CDlgUserInfo(CWnd* pParent = NULL, LLONG hLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgUserInfo)
	enum { IDD = IDD_DLG_USERINFO };
	CListCtrl	m_lsUserInfo;
	CComboBox	m_cmbType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUserInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgUserInfo)
	virtual BOOL OnInitDialog();
	afx_msg void OnUserinfoGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	InitDlg(BOOL bGetUserInfo = TRUE);
	void	GetInfo(BOOL bGetUserInfo = TRUE);
	void	InsertUserInfo(const USER_INFO_NEW& stuInfo);
	void	InsertActiveUserInfo(const CFG_ACTIVEUSER_INFO& stuInfo);
	
private:
	LLONG		m_hLoginId;
	BOOL		m_bGetUserInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGUSERINFO_H__4684D34C_D99F_49A9_924E_18F66A550EF8__INCLUDED_)
