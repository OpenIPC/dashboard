#pragma once

#include "PictureCtrl.h"
// CShowPictureDlg 对话框

class CShowPictureDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowPictureDlg)

public:
	CShowPictureDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowPictureDlg();

// 对话框数据
	enum { IDD = IDD_SHOWPIC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	void ShowPicture(BYTE* data, DWORD size);
	afx_msg void OnStnClickedStaticShowpic();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

private:
	PictureCtrl m_showPicture;
};
