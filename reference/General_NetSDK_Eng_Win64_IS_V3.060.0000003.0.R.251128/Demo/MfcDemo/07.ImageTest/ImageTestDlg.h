// ImageTestDlg.h : header file
//

#if !defined(AFX_IMAGETESTDLG_H__5106A4FE_C43E_402E_99C8_7899D51433EC__INCLUDED_)
#define AFX_IMAGETESTDLG_H__5106A4FE_C43E_402E_99C8_7899D51433EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CImageTestDlg dialog

class CImageTestDlg : public CDialog
{
// Construction
public:
	CImageTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CImageTestDlg)
	enum { IDD = IDD_IMAGETEST_DIALOG };
	CComboBox	m_Cmb_AlgorithmType;
	CComboBox	m_Cmb_Stable;
	CComboBox	m_Cmb_LightMode;
	CComboBox	m_Cmb_DayNight;
	CSliderCtrl	m_Slider_Enhancement;
	CSliderCtrl	m_Slider_Saturation;
	CSliderCtrl	m_Slider_Gamma;
	CSliderCtrl	m_Slider_Contrast;
	CSliderCtrl	m_Slider_Bright;
	CComboBox	m_Cmb_Mode;
	CComboBox	m_Cmb_Ratate;
	CComboBox	m_Cmb_ChannelCount;
    CComboBox   m_Cmb_Type;
    CComboBox   m_Cmb_VideoNoise;
    CComboBox   m_Cmb_SharpnessMode;
	CIPAddressCtrl	m_DvrIPAddr;
	CString	m_DvrPassword;
	CString	m_DvrUserName;
	int		m_DvrPort;
	CString	m_szChannelTitle;
	int	m_szAudioIn;
	int	m_szAudioOut;
	CString	m_Static_Contrast;
	CString	m_Static_Gamma;
	CString	m_Static_Saturation;
	CString	m_Static_Bright;
	BOOL	m_bEnhancement;
	int		m_nSensitivity;
	int		m_nDelay;
	int		m_nCorrection;
	int		m_nSensitive;
	BOOL	m_bIrisAuto;
    int     m_nLevel;
    int     m_nSharpness;
    int     m_nVideoNoise;
	BOOL	m_b2D;
	int		m_n2DLevel;
	int		m_nSnfLevel;
	int		m_nTnfLevel;
	int		m_nRadio;
	int		m_nEndHour;
	int		m_nEndMinute;
	int		m_nEndSecond;
	int		m_nStartHour;
	int		m_nStartMinute;
	int		m_nStartSecond;
	//}}AFX_DATA
    CString GetDvrIP();
    void ShowLoginErrorReason(int nError);
    void AddChannelCount(int nChannelCount);
    void InitDlg();
    void InitStu();
    BOOL GetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType,NET_EM_CONFIG_TYPE EmConfigType = NET_EM_CONFIG_DAYTIME);
    BOOL SetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType,NET_EM_CONFIG_TYPE EmConfigType = NET_EM_CONFIG_DAYTIME);
    void StuToDlg(NET_EM_CFG_OPERATE_TYPE emOperateType);
    void DlgToStu(NET_EM_CFG_OPERATE_TYPE emOperateType);
    void GetAllConfigByType(NET_EM_CONFIG_TYPE EmConfigType);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
    
// Implementation
protected:
	HICON m_hIcon;
    LLONG m_LoginID;
    LLONG m_DispHanle;
    int m_nChannel;             //当前正在播放的通道号
    NET_VIDEOIN_IMAGE_INFO      m_stuVideoInImage;
    NET_ENCODE_CHANNELTITLE_INFO    m_stuChannelTitle;
    NET_AUDIOIN_DENOISE_INFO    m_stuDeNoise;
    NET_AUDIOIN_VOLUME_INFO     m_stuAudioInVolume;
    NET_AUDIOOUT_VOLUME_INFO    m_stuAudioOutVolume;
    NET_VIDEOIN_SWITCH_MODE_INFO m_stuVideoMode;
    NET_VIDEOIN_COLOR_INFO      m_stuVideoColor;
    NET_VIDEOIN_IMAGEENHANCEMENT_INFO m_stuImageEnhancement;
    NET_VIDEOIN_DAYNIGHT_INFO   m_stuDayNight;
    NET_VIDEOIN_LIGHTING_INFO   m_stuLighting;
    NET_VIDEOIN_STABLE_INFO     m_stuStable;
    NET_VIDEOIN_IRISAUTO_INFO   m_stuIrisAuto;
    NET_VIDEOIN_SHARPNESS_INFO  m_stuSharpness;
    NET_VIDEOIN_DENOISE_INFO    m_stuVideoDenoise;
    NET_VIDEOIN_3D_DENOISE_INFO m_stuVideo3DDenoise;
    NET_VIDEOIN_DAYNIGHT_ICR_INFO m_stuICR;
	// Generated message map functions
	//{{AFX_MSG(CImageTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();
	afx_msg void OnSelchangeComboChannelcount();
	afx_msg void OnSharpGet();
	afx_msg void OnSharpSet();
	afx_msg void OnBtnGetchanneltitle();
	afx_msg void OnBtnSetchanneltitle();
	afx_msg void OnBtnGetaudio();
	afx_msg void OnBtnSetaudio();
	afx_msg void OnBtnGetmode();
	afx_msg void OnBtnSetmode();
	afx_msg void OnBtnMinusBright();
	afx_msg void OnBtnPlusBright();
	afx_msg void OnBtnMinusContrase();
	afx_msg void OnBtnPlusContrase();
	afx_msg void OnBtnMinusSaturation();
	afx_msg void OnBtnPlusSaturation();
	afx_msg void OnBtnMinusGamma();
	afx_msg void OnBtnPlusGamma();
	afx_msg void OnBtnGetvideocolor();
	afx_msg void OnBtnSetvideocolor();
	afx_msg void OnCustomdrawSliderBright(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderContrase(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderGamma(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnMinusEnhancement();
	afx_msg void OnBtnPlusEnhancement();
	afx_msg void OnCustomdrawSliderEnhancement(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnGetenhancement();
	afx_msg void OnBtnSetenhancement();
	afx_msg void OnBtnGetdaynight();
	afx_msg void OnBtnSetdaynight();
	afx_msg void OnBtnGetlighting();
	afx_msg void OnBtnSetlighting();
	afx_msg void OnBtnGetstable();
	afx_msg void OnBtnSetstable();
	afx_msg void OnBtnGetiris();
	afx_msg void OnBtnSetiris();
	afx_msg void OnBtnBlacklight();
	afx_msg void OnBtnDefog();
	afx_msg void OnBtnExposure();
	afx_msg void OnBtnWhitebalance();
    afx_msg void OnBtnGetsharpness();
    afx_msg void OnBtnSetsharpness();
    afx_msg void OnSelchangeComboLightmode();
    afx_msg void OnBtnGetvideonoise();
    afx_msg void OnBtnSetvideonoise();
    afx_msg void OnSelchangeCmbType();
	afx_msg void OnBtnAudiotype();
	afx_msg void OnFocusvalue();
	afx_msg void OnSelchangeComboAlgorithmtype();
	afx_msg void OnSelchangeComboVideonoise();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGETESTDLG_H__5106A4FE_C43E_402E_99C8_7899D51433EC__INCLUDED_)
