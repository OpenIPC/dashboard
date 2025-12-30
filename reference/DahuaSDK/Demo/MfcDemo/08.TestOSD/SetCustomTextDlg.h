#pragma once


// CSetCustomTextDlg 对话框

class CSetCustomTextDlg : public CDialog
{
	DECLARE_DYNAMIC(CSetCustomTextDlg)

public:
	CSetCustomTextDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetCustomTextDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_CUSTOM_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonSaveText();
    virtual BOOL OnInitDialog();

    void UpdateCustomText();
    void UpdateCustomTextAlign();

    void EnableCtrl(UINT uID,BOOL bEnable = TRUE);
    void EnableCtrls(int arIDs[],int iNum,BOOL bEnable = TRUE);
    afx_msg void OnBnClickedButtonGet();
    afx_msg void OnCbnSelchangeComboCover();
};
