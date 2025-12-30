#pragma once


// CGetTemper 对话框

class CGetTemper : public CDialog
{
	DECLARE_DYNAMIC(CGetTemper)

public:
	//CGetTemper(CWnd* pParent = NULL);   // 标准构造函数
	CGetTemper(CWnd* pParent = NULL,LLONG iLoginId = 0,int nChannelCount = 0);   // standard constructor

	virtual ~CGetTemper();

// 对话框数据
	enum { IDD = IDD_DLG_GETTEMPER };
	int		m_nX1; // [0, 8191]
	int		m_nY1; // [0, 8191]
	int		m_nX2; // [0, 8191]
	int		m_nY2; // [0, 8191]
	int		m_nX3; // [0, 8191]
	int		m_nY3; // [0, 8191]
	int		m_nX4; // [0, 8191]
	int		m_nY4; // [0, 8191]

	CString	m_unit;
	double m_dbTemperAver;
	double m_dbTemperMax;
	double m_dbTemperMin;

	int		m_nTemperMaxPointX;
	int		m_nTemperMaxPointY;
	int		m_nTemperMinPointX;
	int		m_nTemperMinPointY;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	LLONG m_lLoginID;
	int m_nChannelCount;
	int m_nChannel;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedGettemper();

	CComboBox m_combo_ch;
};
