#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "PictureCtrl.h"

struct tagRecordInfo
{
	int				nRecNo;
	char			szCardNo[DH_MAX_CARDNO_LEN];            // 卡号
	char            szUserID[DH_MAX_USERID_LEN];            // 用户ID

	NET_TIME        stuTime;                                // 刷卡时间
	BOOL            bStatus;                                // 刷卡结果,TRUE表示成功,FALSE表示失败
	NET_ACCESSCTLCARD_TYPE      emCardType;                 // 卡类型
	char           szSnapFtpUrl[MAX_PATH];				// 刷卡抓拍图片的地址

	NET_ACCESS_DOOROPEN_METHOD		emMethod;                 // 开门方式
	
};


// CRecordManagerDlg 对话框

class CRecordManagerDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecordManagerDlg)

public:
	CRecordManagerDlg(LLONG lLoginID, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRecordManagerDlg();

// 对话框数据
	enum { IDD = IDD_RECORDMANAGER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnRecordSearch();
	afx_msg void OnBnClickedRecordCardnoCheck();
	afx_msg void OnBnClickedCheckQuerytime();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	void	DoFindNextRecord();

private:
	void	InitControl();
	void	CloseFindhandle();
	void	CloseThread();
	void	ClearRecordVector();
	void	UpdateLcRecordInfo(int nBeginNum, int nShowNum);

	void	CopyRecordInfo(tagRecordInfo *pRecordInfoDest, NET_RECORDSET_ACCESS_CTL_CARDREC *pReocrdInfoSrc);

	LRESULT DoThreadFindRecord(WPARAM wParam, LPARAM lParam);
	LRESULT DoCloseThread(WPARAM wParam, LPARAM lParam);

	void		SetDate(NET_TIME &stNetTime, int nID);
	void		SetTime(NET_TIME &stNetTime, int nID);
	

private:
	LLONG	m_lLoginID;
	LLONG	m_lFindHandle;

	BOOL	m_bIsDoFindNext;
	HANDLE	m_ThreadHandle;

	CRITICAL_SECTION m_csRecordVector;
	std::vector<tagRecordInfo*> m_recordVector;

private:
	CListCtrl m_lcRecordList;
	CButton m_btnCardNoCheck;
	CButton m_btnQeuryTimeCheck;
public:
	afx_msg void OnBnClickedBtnRecordDelete();
	afx_msg void OnBnClickedBtnRecordClear();
	afx_msg void OnLvnItemchangedRecordList(NMHDR *pNMHDR, LRESULT *pResult);
	PictureCtrl m_recordPic;
	afx_msg void OnBnClickedBtnPicDownload();
};
