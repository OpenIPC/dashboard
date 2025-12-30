#if !defined(AFX_USER_ADDGROUP_H__0BD0B725_BFA2_4587_9073_C84FF1D2A231__INCLUDED_)
#define AFX_USER_ADDGROUP_H__0BD0B725_BFA2_4587_9073_C84FF1D2A231__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// User_AddGroup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUser_AddGroup dialog

class CUser_AddGroup : public CDialog
{
// Construction
public:
	CUser_AddGroup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUser_AddGroup)
	enum { IDD = IDD_USER_ADD_GROUP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUser_AddGroup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUser_AddGroup)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetEnvrmt(USER_MANAGE_INFO_NEW *info, DeviceNode *dev);

private:
	USER_MANAGE_INFO_NEW* m_user_info;
	DeviceNode* m_dev;
	CTreeCtrl m_rightList;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USER_ADDGROUP_H__0BD0B725_BFA2_4587_9073_C84FF1D2A231__INCLUDED_)
