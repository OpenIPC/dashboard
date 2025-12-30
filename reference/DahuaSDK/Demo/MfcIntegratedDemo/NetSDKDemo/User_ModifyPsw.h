#if !defined(AFX_USER_MODIFYPSW_H__3265C3E8_01D4_40E7_BBE4_04514D817845__INCLUDED_)
#define AFX_USER_MODIFYPSW_H__3265C3E8_01D4_40E7_BBE4_04514D817845__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// User_ModifyPsw.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyPsw dialog

class CUser_ModifyPsw : public CDialog
{
// Construction
public:
	CUser_ModifyPsw(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUser_ModifyPsw)
	enum { IDD = IDD_USER_MODIFY_PSW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUser_ModifyPsw)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUser_ModifyPsw)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD userIdx, DeviceNode *dev);
	
private:
	USER_MANAGE_INFO_NEW    *m_user_info;
	DeviceNode              *m_dev;
	DWORD                   m_nUserIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USER_MODIFYPSW_H__3265C3E8_01D4_40E7_BBE4_04514D817845__INCLUDED_)
