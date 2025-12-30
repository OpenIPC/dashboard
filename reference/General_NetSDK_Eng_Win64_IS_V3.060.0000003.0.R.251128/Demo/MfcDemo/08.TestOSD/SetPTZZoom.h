#pragma once


// CSetPTZZoom 对话框

class CSetPTZZoom : public CDialog
{
	DECLARE_DYNAMIC(CSetPTZZoom)

public:
	CSetPTZZoom(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetPTZZoom();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGet();
	int m_nLeft; // [0, 8191]
	int m_nTop;  // [0, 8191]
	int m_nTime;
	BOOL m_bMainBlend;
	BOOL m_bPreviewBlend;
	NET_OSD_PTZZOOM_INFO m_stuPTZZoom;
	afx_msg void OnBnClickedSet();
};
