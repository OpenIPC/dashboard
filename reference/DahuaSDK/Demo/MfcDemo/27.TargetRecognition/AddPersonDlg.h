#include "afxwin.h"
#if !defined(AFX_ADDPERSONDLG_H__6B872F75_CC1D_4790_9EF0_6497171D4F9B__INCLUDED_)
#define AFX_ADDPERSONDLG_H__6B872F75_CC1D_4790_9EF0_6497171D4F9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddPersonDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddPersonDlg dialog

class CAddPersonDlg : public CDialog
{
// Construction
public:
	CAddPersonDlg(const LLONG lLoginHandle, const char *pszGroupID, const char *pszGroupName, const int nOpreateType = 0, CANDIDATE_INFOEX *pstCandidateInfo = NULL, CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddPersonDlg();
// Dialog Data
	//{{AFX_DATA(CAddPersonDlg)
	enum { IDD = IDD_ADD_PERSON_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddPersonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAddPersonDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnOpenPic();
	afx_msg void OnBnClickedOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	FACERECOGNITION_PERSON_INFOEX*	GetAddPersonInfo() {return &m_stAddPersonInfo;}

	CComboBox m_cbSexType;
	CComboBox m_cbCardType;
	char m_szFilePath[512];
	char m_szGroupId[64];
	char m_szGroupName[64];
	char m_szUID[DH_MAX_PERSON_ID_LEN];
	LLONG m_lLoginID;
	int m_nOpreateType;
	CDateTimeCtrl	m_BirthDay;
	CANDIDATE_INFOEX m_stCandiDateInfo;
	FACERECOGNITION_PERSON_INFOEX m_stAddPersonInfo;
	CPictureShow m_pPersonAddPic;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDPERSONDLG_H__6B872F75_CC1D_4790_9EF0_6497171D4F9B__INCLUDED_)
