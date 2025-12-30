#pragma once
#include "afxwin.h"
#include "PictureCtrl.h"


// CShowPictureDlg 对话框

class CShowPictureDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPictureDlg)

public:
	CShowPictureDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowPictureDlg();

// 对话框数据
	enum { IDD = IDD_PICTURE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	PictureCtrl m_showPicture;

public:
	void ShowPicture(BYTE* data, DWORD size);
	afx_msg void OnStnClickedStaticShowpic();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
