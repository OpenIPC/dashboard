#pragma once

#include "AttendanceManager.h"
// CFingerprintCollectionDlg 对话框

#define WM_ACCESS_FINGERPRINT	(WM_USER+205)

class CFingerprintCollectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CFingerprintCollectionDlg)

public:
	CFingerprintCollectionDlg(CAttendanceManager *pManager, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFingerprintCollectionDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_FINGERPRINT_COLLECTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg LRESULT OnFingerPrintMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	bool GetFingerPrintData(char *pFingerBuf, int nBufLen);
	int GetFingerPrintLen();
private:
	CAttendanceManager *m_pManager;
	bool m_bListened;
	char m_byFingerprintData[6*1024];
	int m_nFingerprintLen;
	bool m_bCaptureSuc;
	bool m_bStarted;
	bool m_bFinished;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonStartCollection();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
