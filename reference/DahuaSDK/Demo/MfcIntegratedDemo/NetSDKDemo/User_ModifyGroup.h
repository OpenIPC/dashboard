#if !defined(AFX_USER_MODIFYGROUP_H__63026D29_70BB_4892_A55A_313B4C874C7B__INCLUDED_)
#define AFX_USER_MODIFYGROUP_H__63026D29_70BB_4892_A55A_313B4C874C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// User_ModifyGroup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyGroup dialog

class CUser_ModifyGroup : public CDialog
{
// Construction
public:
	CUser_ModifyGroup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUser_ModifyGroup)
	enum { IDD = IDD_USER_MODIFY_GROUP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUser_ModifyGroup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUser_ModifyGroup)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD gpIdx, DeviceNode *dev);
	void RefreshRightList(DWORD *dwList);
	
private:
	USER_MANAGE_INFO_NEW *m_user_info;
	DeviceNode *m_dev;
	DWORD m_nGroupIndex;
	CTreeCtrl m_rightList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USER_MODIFYGROUP_H__63026D29_70BB_4892_A55A_313B4C874C7B__INCLUDED_)
