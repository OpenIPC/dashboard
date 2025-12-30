#pragma once
#include "afxwin.h"


// PTZDlg 对话框

class PTZDlg : public CDialog
{
	DECLARE_DYNAMIC(PTZDlg)

public:
	PTZDlg(CWnd* pParent = NULL, LLONG lLoginId = 0, int nChannelCount = 0);   // 标准构造函数
	virtual ~PTZDlg();

// 对话框数据
	enum { IDD = IDD_DIG_PTZ };

protected:
	LLONG m_lLoginID;
	int m_nChannelCount;
	bool bFirstinit;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonSetpangroupscan();
	afx_msg void OnBnClickedButtonGetpangroupscan();
	afx_msg void OnBnClickedButtonSetpangroup();
	afx_msg void OnBnClickedButtonGetpangroup();
	afx_msg void OnBnClickedButtongotopanpos();
	afx_msg void OnBnClickedButtonSetpangrouplimit();
	afx_msg void OnBnClickedButtonpause();
	afx_msg void OnBnClickedButtonresume();
	afx_msg void OnCbnSelchangeComboGroup();

	CComboBox m_combo_ch1;
	CComboBox m_combo_ch2;
	CComboBox m_combo_ch3;
	CComboBox m_combo_limitmode;
	CComboBox m_combo_setlimitmode;
	CButton m_check_limit;
	CComboBox m_combo_group;
	CButton m_check_groupenable;
	CComboBox m_combo_index;
	CEdit m_ctrlname;

	CString m_group_name;
	CString m_strgroup_speed;
	CString m_strgroup_interval;
	
	NET_OUT_GET_PAN_GROUP_PARAM stuOutGetPanGroupParam;
	CComboBox m_combo_index2;
	CComboBox m_combo_ch4;
};
