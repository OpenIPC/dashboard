// AttendanceDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "CAttendanceObserver.h"

class CAttendanceManager;

// CAttendanceDlg 对话框
class CAttendanceDlg : public CDialog, CAttendanceObserver
{
// 构造
public:
	CAttendanceDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ATTENDANCE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
	virtual int updateAttendanceInfo(ATTENDANCE_MSG_TYPE msgType, void *pMsgInfo, BYTE *pBuffer, DWORD dwBufSize);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnAccessCtlEvent(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_listCtlUserInfo;
	afx_msg void OnLvnItemchangedListUserInfo(NMHDR *pNMHDR, LRESULT *pResult);
	int InitUserListCtl(void);
	CListCtrl m_EventListCtl;
	int InitEventListCtl(void);
	CIPAddressCtrl m_ipCtrl;
	afx_msg void OnBnClickedButtonLogin();
	int m_nPort;
	CString m_strLoginUser;
	CString m_strLoginPasswd;
	bool m_bLogined;   //是否已经登陆上了
	bool m_bSubscribed;  //是否订阅过了
	int initLoginParam(void);
	CAttendanceManager *m_pAttendanceManager;
public:
	afx_msg void OnDestroy();
private:
	void ShowLoginErrorReason(int nerror);
	void UpdateUserList(void *pUserList, int nUserCount, int nTotalCount, bool bUpdateTotalCount);
public:
	afx_msg void OnBnClickedButtonAdduser();
private:
	int EnableButtons(bool bEnable);
public:
	afx_msg void OnBnClickedButtonModifyuser();
	afx_msg void OnBnClickedButtonDeleteuser();
	afx_msg void OnBnClickedButtonGetuser();
	afx_msg void OnBnClickedEditSubscribe();
	afx_msg void OnBnClickedButtonFinduser();
private:
	int m_nOffset;
	int m_nQueryCount;
	int m_nTotal;
	CString m_strGetUser_USERID;
	int m_nEventIndex;
	void RefreshUI(void);
	int m_nFindUserCount;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonOperateFingerprintByuserid();
	afx_msg void OnBnClickedButtonOperateFingerprintByfpid();
};
