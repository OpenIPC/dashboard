#pragma once
#include "AttendanceManager.h"
#include "afxwin.h"
// CFingerprintByFPID 对话框

class CFingerprintByFPID : public CDialog
{
	DECLARE_DYNAMIC(CFingerprintByFPID)

public:
	CFingerprintByFPID(CAttendanceManager *pManager, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFingerprintByFPID();

// 对话框数据
	enum { IDD = IDD_DIALOG_OPERATE_FINGER_BY_FINGERID };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGetbyfingerprintid();
	virtual BOOL OnInitDialog();
private:
	CAttendanceManager *m_pManager;
	CString m_strUserID;
public:
	afx_msg void OnBnClickedButtonDeleteByFingerprintid();
private:
	CEdit m_Edit_FingerprintData;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
private:
	CString m_strFingerID;
};
