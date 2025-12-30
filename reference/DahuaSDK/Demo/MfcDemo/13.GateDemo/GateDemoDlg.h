// GateDemoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "PictureCtrl.h"

// CGateDemoDlg dialog
class CGateDemoDlg : public CDialog
{
// Construction
public:
	CGateDemoDlg(CWnd* pParent = NULL);	// standard constructor
	~CGateDemoDlg();

// Dialog Data
	enum { IDD = IDD_GATEDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	void	InitSdk();
	void	ShowLoginErrorReason(int nError);
	void	CloseLoginHandle();
	void	CloseAttchHandle();
	void	ClearEventInfo();
	LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlarmCome(WPARAM wParam, LPARAM lParam);

	void	DealWithEvent(LLONG lAnalyzerHandle, DWORD dwAlarmType, char* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, int nSequence);

private:
	LLONG	m_lLoginID;
	LLONG	m_lAttchID;
	int		m_nChannelNum;
public:
	afx_msg void OnBnClickedBtnCardOperate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnLogin();
	CComboBox m_cbxChannel;
	PictureCtrl m_EventPic;
	afx_msg void OnBnClickedBtnLogout();
	afx_msg void OnBnClickedBtnAttachevent();
	afx_msg void OnBnClickedBtnDetachevent();
	afx_msg void OnBnClickedBtnRecordOperate();
};
