#if !defined(AFX_ADDGROUPDLG_H__F8F7ACB7_B577_452D_9B6A_5538A00706A6__INCLUDED_)
#define AFX_ADDGROUPDLG_H__F8F7ACB7_B577_452D_9B6A_5538A00706A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddGroupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddGroupDlg dialog

class CAddGroupDlg : public CDialog
{
// Construction
public:
	CAddGroupDlg(const LLONG lLoginHandle, const int nOpreateType = 0, const NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo = NULL, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddGroupDlg)
	enum { IDD = IDD_ADDGROUP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddGroupDlg)
	protected:
	virtual BOOL CAddGroupDlg::OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddGroupDlg)
	virtual void OnCancel();
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public :
	char *GetEditGroupName() {return m_szGroupName;}

private:
	LLONG	m_lLoginID;
	char	m_szGroupName[DH_COMMON_STRING_64];
	int		m_nOpreateType;
	NET_FACERECONGNITION_GROUP_INFO m_stuGroupInfo;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDGROUPDLG_H__F8F7ACB7_B577_452D_9B6A_5538A00706A6__INCLUDED_)
