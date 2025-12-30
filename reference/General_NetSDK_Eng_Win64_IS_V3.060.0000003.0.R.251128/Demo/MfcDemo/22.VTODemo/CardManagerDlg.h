#pragma once


// CCardManagerDlg 对话框
#include "CardInfo.h"
#include "afxcmn.h"
#include "CMyListCtl.h"

#include <vector>

class CCardManagerDlg : public CDialog
{
	DECLARE_DYNAMIC(CCardManagerDlg)

public:
	CCardManagerDlg(LLONG lLoginID, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCardManagerDlg();

// 对话框数据
	enum { IDD = IDD_CARDMANAGER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	LLONG m_lLoginHandle;
	BOOL	m_bIsDoFindNext;
	HANDLE  m_ThreadHandle;
	LLONG	m_lFindHandle;
	DWORD   m_dwThreadID;
	CRITICAL_SECTION m_csCardInfoVector;
	std::vector<tagCardInfo*> m_cardInfoVector;
	std::vector<int> m_nRecNoVec;

public:
	afx_msg void OnBnClickedBtnCardsearch();
	afx_msg void OnBnClickedBtnCardadd();
	afx_msg void OnBnClickedBtnCardmodify();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnBnClickedBtnClear();

	void	DoFindNextRecord();
	void	CopyCardInfo(tagCardInfo *pCardInfoDest, NET_RECORDSET_ACCESS_CTL_CARD *pCardInfoSrc);
	void	UpdateLcCardInfo(int nBeginNum, int nShowNum);

private:
	LRESULT DoThreadFindCard(WPARAM wParam, LPARAM lParam);
	LRESULT DoCloseThread(WPARAM wParam, LPARAM lParam);
	void	CloseThread();
	void	ClearCardInfoVector();
	void	CloseFindhandle();
	BOOL	DeleteCardInfo(tagCardInfo *pCardInfo);
	BOOL	DeleteFaceInfo(const tagCardInfo& cardInfo);
	BOOL	ClearFaceInfo();
	BOOL	ClearCardInfo();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	CMyListCtl m_listCtrl;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
};
