#include "afxwin.h"
#if !defined(AFX_SEARCHGROUPGLG_H__89BE0B20_DD39_41EA_9FBE_100F55B664C3__INCLUDED_)
#define AFX_SEARCHGROUPGLG_H__89BE0B20_DD39_41EA_9FBE_100F55B664C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PersonOpreateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPersonOpreateGlg dialog
typedef struct tagNET_PERSON_LIST_INFO
{
	int				nNumber;
	int				nIndex;
	CANDIDATE_INFOEX	stuCandidate;
}NET_PERSON_LIST_INFO;

class CPersonOpreateGlg : public CDialog
{
// Construction
public:
	CPersonOpreateGlg(const LLONG lLoginHandle, const char* pszGroupId, const char* pszGroupName, char* pstSoftPath, CWnd* pParent = NULL);   // standard constructor
	CPersonOpreateGlg::~CPersonOpreateGlg();

// Dialog Data
	//{{AFX_DATA(CPersonOpreateGlg)
	enum { IDD = IDD_PERSON_OPREATE_DIALOG };


		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPersonOpreateGlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

	CComboBox m_cbSexType;
	CComboBox m_cbCardType;
protected:

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPersonOpreateGlg)
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonPer();
	afx_msg void OnButtonPersonAdd();
	afx_msg void OnButtonPersonEdit();
	afx_msg void OnSelectPersonInfo();
	afx_msg void OnButtonPersonDel();
	afx_msg void OnBnClickedSearch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	ClearShowCandidateInfo();
	void	ShowCandidateInfo(int nCount);
	void    StartFindPersonInfo(BOOL bDelete = FALSE);
	void	SearchPersonInfoFromGroup();
	
	LLONG	m_lLoginID;
	char	m_szGroupId[64];
	char	m_szGroupName[64];
	char	m_szSelectPersonFile[64];
	int		m_nCurPos;
	int		m_nPersonCount;	// total numbers of the current face library
	NET_PERSON_LIST_INFO *m_pPersonInfoList; // Store the information about the face
	NET_PERSON_LIST_INFO *m_pstPersonSelectInfo;
	char *m_pszSoftPath;

	CDateTimeCtrl	m_BirthDayStart;
	CDateTimeCtrl	m_BirthDayEnd;

	LLONG	m_lFindPersonHandle;
public:
	CPictureShow m_pPersonPic;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHGROUPGLG_H__89BE0B20_DD39_41EA_9FBE_100F55B664C3__INCLUDED_)
