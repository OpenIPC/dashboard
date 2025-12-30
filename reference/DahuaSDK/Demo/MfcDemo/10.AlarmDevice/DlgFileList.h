#if !defined(AFX_DLGFILELIST_H__418482AD_95EF_403B_81A8_10171F89A550__INCLUDED_)
#define AFX_DLGFILELIST_H__418482AD_95EF_403B_81A8_10171F89A550__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFileList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgFileList dialog
#include <list>
class CDlgFileList : public CDialog
{
// Construction
public:
	CDlgFileList(CWnd* pParent = NULL, LLONG hLoginId = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgFileList)
	enum { IDD = IDD_DLG_FILENAME };
	CListCtrl	m_lsFileList;
	UINT	m_uiRetCount;
	UINT	m_uiMaxCount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFileList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgFileList)
	virtual BOOL OnInitDialog();
	afx_msg void OnFilenameBtnGet();
	afx_msg void OnFilenameBtnRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg();

private:
	LLONG	m_hLoginId;
	std::list<char*>	m_listInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFILELIST_H__418482AD_95EF_403B_81A8_10171F89A550__INCLUDED_)
