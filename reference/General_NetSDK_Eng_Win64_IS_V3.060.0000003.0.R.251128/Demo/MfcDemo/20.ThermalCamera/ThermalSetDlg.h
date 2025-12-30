#pragma once
#include "afxwin.h"


// ThermalSetDlg 对话框

class ThermalSetDlg : public CDialog
{
	DECLARE_DYNAMIC(ThermalSetDlg)

public:
	ThermalSetDlg(CWnd* pParent = NULL, LLONG lLoginId = 0, int nChannelCount = 0);   // 标准构造函数
	virtual ~ThermalSetDlg();

// 对话框数据
	enum { IDD = IDD_DIG_THERMALSET };

protected:
	LLONG m_lLoginID;
	int m_nChannelCount;
	bool bFirstinit;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGetffc();
	afx_msg void OnBnClickedButtonSetffc();
	afx_msg void OnBnClickedButtonGetfusion();
	afx_msg void OnBnClickedButtonSetfusion();
	afx_msg void OnBnClickedButtonGetlcestate();
	afx_msg void OnBnClickedButtonSetlcestate();
	afx_msg void OnBnClickedButtonGetnoise();
	afx_msg void OnBnClickedButtonSetnoise();
	afx_msg void OnCbnSelchangeComboNoisescene();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonDoffc();

	CComboBox m_Combo_FFCMode;
	CComboBox m_Combo_fusionmode;
	CComboBox m_combo_ffcchn;
	CComboBox m_combo_lcechn;
	CComboBox m_combo_noisechn;
	CComboBox m_combo_noisescene;
	CButton m_check_noise;

	CString m_strPeriod;
	CString m_strfusionrate;
	CString m_strLCEValue;
	CString m_strHistGramValue;
	CString m_strnoisevalue;
	CFG_THERM_DENOISE stuThermDemoise;
	//double m_speed;
	CString m_strasschn1;
	CString m_strasschn2;
	CString m_strspeed;
};
