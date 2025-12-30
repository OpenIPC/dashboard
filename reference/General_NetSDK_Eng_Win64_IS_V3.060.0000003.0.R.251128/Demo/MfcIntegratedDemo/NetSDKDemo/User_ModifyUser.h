#if !defined(AFX_USER_MODIFYUSER_H__E1D7A431_3219_407A_9E06_D02EAAC57B69__INCLUDED_)
#define AFX_USER_MODIFYUSER_H__E1D7A431_3219_407A_9E06_D02EAAC57B69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// User_ModifyUser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyUser dialog

class CUser_ModifyUser : public CDialog
{
// Construction
public:
	CUser_ModifyUser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUser_ModifyUser)
	enum { IDD = IDD_USER_MODIFY_USER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUser_ModifyUser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUser_ModifyUser)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD userIdx, DeviceNode *dev);
	void RefreshRightList(DWORD *dwList);

private:
	USER_MANAGE_INFO_NEW    *m_user_info;
	DeviceNode              *m_dev;
	DWORD                   m_nUserIndex;
	CTreeCtrl               m_rightList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USER_MODIFYUSER_H__E1D7A431_3219_407A_9E06_D02EAAC57B69__INCLUDED_)
