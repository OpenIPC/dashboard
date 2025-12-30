#pragma once
#include "afxwin.h"

#define WM_ACCESS_FINGERPRINTEX (WM_USER + 600)
// UserInfoGetFingerPrint 对话框

class UserInfoGetFingerPrint : public CDialog
{
	DECLARE_DYNAMIC(UserInfoGetFingerPrint)

public:
	UserInfoGetFingerPrint(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~UserInfoGetFingerPrint();
	void SetDlgHWND(HWND  hParentWndtemp)
	{
		hParentWnd = hParentWndtemp;
	}

	void SetDuressState(BOOL bDuress)
	{
		m_bDuress = bDuress;
	}

	BOOL GetDuressState()
	{
		return m_bDuress;
	}

	const int GetFingerprintData(char* byFingerprintData)
	{
		if (m_nFingerprintLen > 0)
		{
			memcpy(byFingerprintData, m_byFingerprintData, m_nFingerprintLen);
		}
		return m_nFingerprintLen;
	}
// 对话框数据
	enum { IDD = IDD_DIALOG_USERINFO_GETFINGERPRINT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButGetfingerprint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	LRESULT OnFingerPrintMessage(WPARAM wParam, LPARAM lParam);
	bool	m_bCaptureSuc;
	bool	m_bStarted;
	bool	m_bFinished;
	char	m_byFingerprintData[6*1024];
	int		m_nFingerprintLen;

	HWND  hParentWnd;
	BOOL  m_bDuress;
	CButton m_chkDuress;
	afx_msg void OnBnClickedOk();
};
