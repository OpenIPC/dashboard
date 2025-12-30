#if !defined(AFX_ADDUSERINFO_H__03DE1F2B_FA2D_405E_B025_9D661F1B80AF__INCLUDED_)
#define AFX_ADDUSERINFO_H__03DE1F2B_FA2D_405E_B025_9D661F1B80AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddUserInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddUserInfo dialog

class CAddUserInfo : public CDialog
{
// Construction
public:
	//To be called by external to initialize whole dialogux box.  
	void InitAddUserInfoDlg(USER_MANAGE_INFO_NEW *info, DWORD dwID, LLONG hLoginID);
	
	CAddUserInfo(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddUserInfo)
	enum { IDD = IDD_DIALOG_ADD_USER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddUserInfo)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CAddUserInfo)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL IsValid();
	CTreeCtrl	m_treeLevels;
	void InitLevelTree(DWORD dwNum, DWORD *dwList);
	LLONG m_hLoginID;
	DWORD m_dwGroupID;
	USER_MANAGE_INFO_NEW *m_usersInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDUSERINFO_H__03DE1F2B_FA2D_405E_B025_9D661F1B80AF__INCLUDED_)
