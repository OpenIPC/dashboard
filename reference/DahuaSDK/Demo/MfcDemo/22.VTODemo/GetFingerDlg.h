#pragma once


// CGetFingerDlg 对话框

#include "../../../Include/Common/dhnetsdk.h"

#define WM_ACCESS_FINGERPRINT	(WM_USER+205)

class CGetFingerDlg : public CDialog
{
	DECLARE_DYNAMIC(CGetFingerDlg)

public:
	CGetFingerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGetFingerDlg();


// 对话框数据
	enum { IDD = IDD_GetFinger_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	LLONG	m_lLoginHandle;
	bool	m_bCaptureSuc;
	bool	m_bStarted;
	bool	m_bFinished;
	char	m_byFingerprintData[6*1024];
	int		m_nFingerprintLen;
	bool	m_bListened;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnLogoutFeinger();
	afx_msg void OnBnClickedBtnLoginFeinger();
	afx_msg void OnBnClickedBtnCollection();
	LRESULT OnFingerPrintMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnCollectiondone();
	afx_msg void OnDestroy();
	bool GetFingerPrintData(char *pFingerBuf, int nBufLen);
	int GetFingerPrintLen();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
