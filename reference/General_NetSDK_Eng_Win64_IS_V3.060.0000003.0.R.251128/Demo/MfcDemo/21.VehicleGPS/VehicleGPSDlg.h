// VehicleGPSDlg.h : header file
//

#pragma once


// CVehicleGPSDlg dialog
class CVehicleGPSDlg : public CDialog
{
// Construction
public:
	CVehicleGPSDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CVehicleGPSDlg();

// Dialog Data
	enum { IDD = IDD_VEHICLEGPS_DIALOG };

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
	afx_msg void OnDestroy();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();
	afx_msg void OnBnClickedRadio();
	afx_msg void OnBnClickedBtnStartlisten();
	afx_msg void OnBnClickedBtnStoplisten();
	afx_msg void OnBnClickedBtnAttachevent();
	afx_msg void OnBnClickedBtnDetachevent();
	afx_msg void OnBnClickedBtnListenLogout();

	LRESULT OnDeviceDisConnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeviceReconnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnGPSInfo(WPARAM wParam, LPARAM lParam);
	LRESULT OnLoginDisconnected(WPARAM wParam, LPARAM lParam);
	LRESULT	OnLoginDevice(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	void InitDlgItem();
	void InitNetSDK();
	void UnInitNetSDK();
	void ShowLoginErrorReason(int nError);
	void DealListenCB(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, HWND hwnd);
	CString ChangeToLONLAT(double dParam, int nType);

private:
	LLONG	m_loginID;
	BOOL	m_bRadio;
	LLONG   m_lListenHdl;
	CString	m_strListenDevID;
	CString	m_strListenDevName;
	CString	m_strListenDevUser;
	CString	m_strListenDevPwd;
	BOOL	m_bLogined;
	
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
