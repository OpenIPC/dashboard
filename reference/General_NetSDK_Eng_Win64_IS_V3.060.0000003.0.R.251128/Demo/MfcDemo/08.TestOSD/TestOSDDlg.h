// TestOSDDlg.h : 头文件
//

#pragma once

extern HANDLE g_hConsole;

struct ComboItemInfo
{
    const char* szText;
    int iValue;
};

void InitCombo(CDialog* pDlg,UINT uComboID,ComboItemInfo arInfo[],int iCount);
void SetComboWithValue(CDialog* pDlg,UINT uComboID,unsigned int iValue);


// CTestOSDDlg 对话框
class CTestOSDDlg : public CDialog
{
// 构造
public:
	CTestOSDDlg(CWnd* pParent = NULL);	// 标准构造函数
    ~CTestOSDDlg();

// 对话框数据
	enum { IDD = IDD_TESTOSD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonLogin();
    afx_msg void OnStnDblclickStaticPic();
//    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
    afx_msg void OnCbnSelchangeComboOsdType();
    afx_msg void OnBnClickedButtonSaveTime();
    afx_msg void OnIpnFieldchangedRect(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonSaveChannel();
    afx_msg void OnBnClickedButtonSaveText();

    void UpdateChannelTitle();
    void UpdateTimeTitle();
    void UpdateTitleCommonInfo();
    afx_msg void OnBnClickedButtonSaveCommoninfo();
    afx_msg void OnBnClickedButtonSetCustomText();
    afx_msg void OnCbnSelchangeComboChannel();

	void ShowLoginErrorReason(int nError);
	afx_msg void OnBnClickedBtnPtzzoom();
    afx_msg void OnBnClickedButtonGetChannel();
    afx_msg void OnBnClickedButtonGetTime();
    afx_msg void OnBnClickedButtonGetCommoninfo();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
};
