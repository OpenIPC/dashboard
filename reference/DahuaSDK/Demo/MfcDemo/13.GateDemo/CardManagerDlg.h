#pragma once
#include "afxcmn.h"
#include "CardInfo.h"

// CCardManagerDlg 对话框

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

public:
	void	DoFindNextRecord();

private:
	void	InitControl();
	void	CloseFindhandle();
	
	void	UpdateLcCardInfo(int nBeginNum, int nShowNum);

	void	CopyCardInfo(tagCardInfo *pCardInfoDest, NET_RECORDSET_ACCESS_CTL_CARD *pCardInfoSrc);
	void	ClearCardInfoVector();

	BOOL	DeleteFaceInfo(const tagCardInfo& cardInfo);
	BOOL	DeleteCardInfo(tagCardInfo *pCardInfo);

	BOOL	ClearFaceInfo();
	BOOL	ClearCardInfo();

	LRESULT DoThreadFindCard(WPARAM wParam, LPARAM lParam);
	LRESULT DoCloseThread(WPARAM wParam, LPARAM lParam);
	void	CloseThread();

private:
	LLONG	m_lLoginID;
	LLONG	m_lFindHandle;
	HANDLE  m_ThreadHandle;
	BOOL	m_bIsDoFindNext;
	DWORD   m_dwThreadID;
	std::vector<tagCardInfo*> m_cardInfoVector;
	CRITICAL_SECTION m_csCardInfoVector;
	
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_lcCardInfo;
	afx_msg void OnBnClickedBtnCardSearch();
	afx_msg void OnBnClickedBtnCardAdd();
	afx_msg void OnBnClickedBtnCardModify();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnCardDelete();
	afx_msg void OnBnClickedBtnClearCard();
};
