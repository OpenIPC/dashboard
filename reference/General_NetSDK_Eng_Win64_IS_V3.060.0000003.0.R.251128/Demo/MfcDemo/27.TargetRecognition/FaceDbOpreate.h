#if !defined(AFX_FACEDBOPREATE_H__28D02C4F_4BA4_43D4_A66D_0F70FC758A0A__INCLUDED_)
#define AFX_FACEDBOPREATE_H__28D02C4F_4BA4_43D4_A66D_0F70FC758A0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaceDbOpreate.h : header file
//

#include "StdAfx.h"

typedef struct tagCOMMON_GROUP_INFO
{
	int		nIndex;
	NET_FACERECONGNITION_GROUP_INFO stuGroupInfo;
}COMMON_GROUP_INFO;

/////////////////////////////////////////////////////////////////////////////
// CFaceDbOpreate dialog

class CFaceDbOpreate : public CDialog
{
	// Construction
public:
	CFaceDbOpreate(LLONG lLoginID, const int nChnCount, char *pSoftPath = NULL, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFaceDbOpreate();
	// Dialog Data
	//{{AFX_DATA(CFaceDbOpreate)
	enum { IDD = IDD_FACEDB_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFaceDbOpreate)
protected:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFaceDbOpreate)
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRefresh();
	afx_msg void OnBUTTONEdit();
	afx_msg void OnButtonPersonOpreate();
	afx_msg void OnButtonDispatch();
	afx_msg void OnSelectGroup();
	afx_msg void OngroupPerPage();
	afx_msg void OngroupNextPage();
	afx_msg void OnBnClickedButtonDispatchChannel();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL GetGroupInfos();
	void ShowGroupInfo();
	void ClearShowGroupInfo();
	void CleanGroupList();
	void RsetGroupInfo();

private:
	LLONG	m_lLoginID;
	char	*m_pSoftPath;		// program path
	int		m_nChnCount;		// total number of channel
	int		m_nGroupCnt;		// total number of group
	int		m_nGroupPos;		// 
	int		m_nSelectId;		// index which select
	std::list<COMMON_GROUP_INFO*>	 m_lstGroupInfo; // the list of group info
	COMMON_GROUP_INFO *m_pstSelectGroup; // the group info wich is selected by user
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FACEDBOPREATE_H__28D02C4F_4BA4_43D4_A66D_0F70FC758A0A__INCLUDED_)
