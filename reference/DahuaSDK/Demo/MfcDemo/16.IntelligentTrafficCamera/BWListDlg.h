#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <vector>

// CBWListDlg Dialog

class CBWListDlg : public CDialog
{
	DECLARE_DYNAMIC(CBWListDlg)

public:
	CBWListDlg(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor
	virtual ~CBWListDlg();

	// Dialog Data
	enum { IDD = IDD_BW_LITS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnQuery();
	afx_msg void OnBnClickedBtnModify();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnBnClickedBtnPreviousPage();
	afx_msg void OnBnClickedBtnNextPage();
	afx_msg void OnCbnSelchangeComboChannelTrafficList();
	DECLARE_MESSAGE_MAP()

public:
	void CleanUp();

private:
	void CleanUpResultOfLastTimeQuery();
	LLONG StartFindTrafficList();
	int GetTotalCountOfTrafficList(LLONG lFindeHandle);
	bool DoFindTrafficList( int nQueryCount );
	void ShowTrafficList(NET_TRAFFIC_LIST_RECORD* pRecordList, int nCurrentIndex);

private:
	CButton     m_btnPreviousPage;
	CButton     m_btnNextPage;
	CButton     m_btnQuery;
	CButton     m_btnAdd;
	CButton     m_btnModify;
	CButton     m_btnDelete;
	CComboBox   m_cmbChannel;
	CComboBox   m_cmbType;
	CListCtrl   m_ctrTrafficList;

	int     m_nCurPage;
	int     m_nTotalPage;
	int     m_nIndexOfTrafficList;
	int     m_nTotalCountOfTrafficList;
	int		m_nChannel;

	LLONG   m_lLoginHandle;
	LLONG   m_LFindeHandle;

	std::vector<NET_TRAFFIC_LIST_RECORD*> m_vecTrafficListInfo;
};
