#if !defined(AFX_BLACKLIGHT_H__579E67CF_9AF3_444C_BAAA_36ED333AB1B7__INCLUDED_)
#define AFX_BLACKLIGHT_H__579E67CF_9AF3_444C_BAAA_36ED333AB1B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BlackLight.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BlackLight dialog

class BlackLight : public CDialog
{
// Construction
public:
	BlackLight(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(BlackLight)
	enum { IDD = IDD_BACKLIGHT_DIALOG };
	CSliderCtrl	m_Slider_Wide;
	CSliderCtrl	m_Slider_Glare;
	CComboBox	m_Cmb_BlackLightMode;
	CComboBox	m_Cmb_BlackMode;
	CComboBox	m_Cmb_Type;
	int		m_nBottom;
	int		m_nLeft;
	int		m_nRight;
	int		m_nTop;
	//}}AFX_DATA
    NET_VIDEOIN_BACKLIGHT_INFO m_stuBlackLight;
    int m_nChannel;
    LLONG m_LoginID;
    BOOL GetVideoIn(NET_EM_CONFIG_TYPE EmConfigType = NET_EM_CONFIG_DAYTIME);
    BOOL SetVideoIn(NET_EM_CONFIG_TYPE EmConfigType = NET_EM_CONFIG_DAYTIME);
    void StuToDlg();
    void DlgToStu();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BlackLight)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
    
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BlackLight)
	virtual BOOL OnInitDialog();
	afx_msg void OnGet();
	afx_msg void OnSet();
	afx_msg void OnSelchangeCmbType();
	afx_msg void OnCustomdrawSliderWide(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderGlare(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnMinusWide();
	afx_msg void OnBtnPlusWide();
	afx_msg void OnBtnMinusGlare();
	afx_msg void OnBtnPlusGlare();
	afx_msg void OnSelchangeComboBlackmode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLACKLIGHT_H__579E67CF_9AF3_444C_BAAA_36ED333AB1B7__INCLUDED_)
