#if !defined(AFX_MODIFYUSERINFO_H__12EA1113_078E_4C66_B571_B4BC16C34BE9__INCLUDED_)
#define AFX_MODIFYUSERINFO_H__12EA1113_078E_4C66_B571_B4BC16C34BE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyUserInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModifyUserInfo dialog

class CModifyUserInfo : public CDialog
{
// Construction
public:
	void InitModifyUserInfoDlg(USER_MANAGE_INFO_NEW *info, DWORD dwID, LLONG hLoginID);
	CModifyUserInfo(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyUserInfo)
	enum { IDD = IDD_DIALOG_MODIFY_USER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyUserInfo)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CModifyUserInfo)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL IsValid();
	void InitLevelTree(DWORD dwNum, DWORD *dwList, DWORD dwGroupNum, DWORD *dwGroupList);
	USER_MANAGE_INFO_NEW *m_usersInfo;
	CTreeCtrl m_treeLevels;
	LLONG m_hLoginID;
	DWORD m_dwUserID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYUSERINFO_H__12EA1113_078E_4C66_B571_B4BC16C34BE9__INCLUDED_)
