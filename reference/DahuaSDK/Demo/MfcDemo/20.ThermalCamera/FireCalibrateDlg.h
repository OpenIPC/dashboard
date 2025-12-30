#pragma once
#include "afxwin.h"


// FireCalibrateDlg 对话框

class FireCalibrateDlg : public CDialog
{
	DECLARE_DYNAMIC(FireCalibrateDlg)

public:
	FireCalibrateDlg(CWnd* pParent = NULL, LLONG lLoginId = 0, int nChannelCount = 0);   // 标准构造函数
	virtual ~FireCalibrateDlg();

// 对话框数据
	enum { IDD = IDD_DIG_CALIBRATE };

protected:
	LLONG m_lLoginID;
	bool bFirstinit;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonGetdevlocation();
	afx_msg void OnBnClickedButtonSetdevlocation();
	afx_msg void OnBnClickedButtonGetcal();
	afx_msg void OnBnClickedButtonSetcal();
	afx_msg void OnBnClickedButtonDelcal();
	afx_msg void OnBnClickedButtonGetallcal();
	afx_msg void OnCbnSelchangeComboPointid();
	afx_msg void OnCbnSelchangeComboCalid();

	double UINTToLONLAT(UINT unParam, int nType);
	UINT LONLATtoUINT(double dParam, int nType);

	CComboBox m_combo_pointid;//参考点

	CString m_name;
// 	double m_dLongitude;
// 	double m_dLatitude;
// 	double m_dAltitude;
	CString m_strx1;
	CString m_stry1;
	CString m_strx2;
	CString m_stry2;
	CComboBox m_combo_calid;//标定点
	
	NET_LOCATION_CALIBRATE_INFO	stuLocationCalibrateInfo;
	CFG_LOCATION_CALIBRATE_INFO stuLocationCalibrateInfoCFG;
// 	double m_dGPSLongitude;
// 	double m_dGPSLatitude;
// 	double m_dGPSAltitude;
// 	float m_fGPSHeight;
	CButton m_check_GPSconfig;
	CString m_strGPSLongitude;
	CString m_strGPSLatitude;
	CString m_strGPSAltitude;
	CString m_strGPSHeight;
	CString m_strLongitude;
	CString m_strLatitude;
	CString m_strAltitude;
};
