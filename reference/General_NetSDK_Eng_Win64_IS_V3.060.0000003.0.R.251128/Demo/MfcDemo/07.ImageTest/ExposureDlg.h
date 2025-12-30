#if !defined(AFX_EXPOSUREDLG_H__88666926_7551_4BA1_8B6A_5FF1BC1502BA__INCLUDED_)
#define AFX_EXPOSUREDLG_H__88666926_7551_4BA1_8B6A_5FF1BC1502BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExposureDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExposureDlg dialog

class CExposureDlg : public CDialog
{
// Construction
public:
	CExposureDlg(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExposureDlg)
	enum { IDD = IDD_EXPOSURE_DIALOG };
	CComboBox	m_Cmb_AntiFlicker;
	CComboBox	m_Cmb_Exposure;
	CComboBox	m_Cmb_Type;
	int		m_nCompensation;
	int		m_nGainMax;
	int		m_nGainMin;
	BOOL	m_bSlowShutter;
	int		m_nSlowExposure;
	int		m_nRecoveryTime;
	int		m_nIrisMax;
	int		m_nIrisMin;
	BOOL	m_bAuto;
	int		m_nExposureIris;
	double	m_fExposureMax;
	double	m_fExposureMin;
	float 	m_fShutter;
	int		m_nPhase;
	int		m_nSlowSpeed;
	int		m_nGain;
	//}}AFX_DATA
    int m_nChannel;
    LLONG m_LoginID;
    BOOL GetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperate,NET_EM_CONFIG_TYPE emConfigType = NET_EM_CONFIG_DAYTIME);
    BOOL SetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperate,NET_EM_CONFIG_TYPE emConfigType = NET_EM_CONFIG_DAYTIME);
    void StuToDlg();
    void DlgToStu();
    NET_VIDEOIN_EXPOSURE_NORMAL_INFO m_stuExposure_Normal;
    NET_VIDEOIN_EXPOSURE_OTHER_INFO  m_stuExposure_Other;
    NET_VIDEOIN_EXPOSURE_SHUTTER_INFO m_stuExposure_Shutter;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExposureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExposureDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGet();
	afx_msg void OnSet();
	afx_msg void OnSelchangeComboType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPOSUREDLG_H__88666926_7551_4BA1_8B6A_5FF1BC1502BA__INCLUDED_)
