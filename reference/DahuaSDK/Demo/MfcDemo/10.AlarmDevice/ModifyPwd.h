#if !defined(AFX_MODIFYPWD_H__8B7A585C_4A39_4DE2_8A0D_C771AC194EB4__INCLUDED_)
#define AFX_MODIFYPWD_H__8B7A585C_4A39_4DE2_8A0D_C771AC194EB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyPwd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModifyPwd dialog

class CModifyPwd : public CDialog
{
// Construction
public:
	void InitModifyPwdInfoDlg(USER_MANAGE_INFO_NEW *info, DWORD dwID, LLONG hLoginID);
	
	CModifyPwd(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyPwd)
	enum { IDD = IDD_DIALOG_MODIFY_PWD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyPwd)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CModifyPwd)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL IsValid();
	DWORD m_dwUserID;
	LLONG m_hLoginID;
	USER_MANAGE_INFO_NEW *m_usersInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYPWD_H__8B7A585C_4A39_4DE2_8A0D_C771AC194EB4__INCLUDED_)
