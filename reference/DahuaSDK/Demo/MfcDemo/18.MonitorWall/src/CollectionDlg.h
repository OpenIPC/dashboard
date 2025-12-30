#if !defined(AFX_COLLECTIONDLG_H__6CDD7A33_AA25_4BFE_ADE9_5E76596FDCC4__INCLUDED_)
#define AFX_COLLECTIONDLG_H__6CDD7A33_AA25_4BFE_ADE9_5E76596FDCC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CollectionDlg.h : header file
//

class CMonitorWallCollections
{
public:
	CMonitorWallCollections(const DH_OUT_WM_GET_COLLECTIONS* pInfo)
	{
		for (int i = 0; i < pInfo->nCollectionsCount; ++i)
		{
			m_vecCollections.push_back(pInfo->pCollections[i]);
		}
	}
	std::vector<DH_MONITORWALL_COLLECTION> m_vecCollections;
};

class CSplitCollections
{
public:
	CSplitCollections(const DH_OUT_SPLIT_GET_COLLECTIONS* pInfo)
	{
		for (int i = 0; i < pInfo->nCollectionsCount; ++i)
		{
			m_vecCollections.push_back(pInfo->pCollections[i]);
		}
	}
	std::vector<DH_BLOCK_COLLECTION> m_vecCollections;
};

/////////////////////////////////////////////////////////////////////////////
// CCollectionDlg dialog

class CCollectionDlg : public CDialog
{
// Construction
public:
	CCollectionDlg(LLONG lLoginID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCollectionDlg)
	enum { IDD = IDD_DIALOG_COLLECTION };
	CComboBox	m_cbWall;
	CListBox	m_lbCollection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCollectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitUI();

	LLONG m_lLoginID;

	// 电视墙预案	
	std::vector<CMonitorWallCollections> m_vecMonitorWallCollections;

	// 分割收藏
	std::map<int, CSplitCollections> m_mapSplitCollections;

	// Generated message map functions
	//{{AFX_MSG(CCollectionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnLoad();
	afx_msg void OnBtnSave();
	afx_msg void OnSelchangeListCollection();
	afx_msg void OnSelchangeComboMonitorwall();
	afx_msg void OnBtnRename();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLLECTIONDLG_H__6CDD7A33_AA25_4BFE_ADE9_5E76596FDCC4__INCLUDED_)
