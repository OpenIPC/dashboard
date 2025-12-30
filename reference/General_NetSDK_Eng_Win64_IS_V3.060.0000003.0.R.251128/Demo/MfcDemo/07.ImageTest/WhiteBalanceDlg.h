#if !defined(AFX_WHITEBALANCEDLG_H__0F0C2EB9_34A1_43E4_BD39_F427AA0BA7CB__INCLUDED_)
#define AFX_WHITEBALANCEDLG_H__0F0C2EB9_34A1_43E4_BD39_F427AA0BA7CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WhiteBalanceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWhiteBalanceDlg dialog

class CWhiteBalanceDlg : public CDialog
{
// Construction
public:
	CWhiteBalanceDlg(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor
    
// Dialog Data
	//{{AFX_DATA(CWhiteBalanceDlg)
	enum { IDD = IDD_WHITEBALANCE_DIALOG };
	CSliderCtrl	m_Slider_Red;
	CSliderCtrl	m_Slider_Green;
	CSliderCtrl	m_Slider_Blue;
	CComboBox	m_Cmb_WhiteBalance;
	CComboBox	m_Cmb_Type;
	int		m_nColorTemperature;
	//}}AFX_DATA
    int m_nChannel;
    LLONG m_LoginID;
    NET_VIDEOIN_WHITEBALANCE_INFO m_stuWhiteBalance;
    BOOL GetVideoIn(NET_EM_CONFIG_TYPE emConfigType = NET_EM_CONFIG_DAYTIME);
    BOOL SetVideoIn(NET_EM_CONFIG_TYPE emConfigType = NET_EM_CONFIG_DAYTIME);
    void StuToDlg();
    void DlgToStu();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhiteBalanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWhiteBalanceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGet();
	afx_msg void OnSet();
	afx_msg void OnBtnMinusRed();
	afx_msg void OnBtnMinusBlue();
	afx_msg void OnBtnMinusGreen();
	afx_msg void OnBtnPlusRed();
	afx_msg void OnBtnPlusBlue();
	afx_msg void OnBtnPlusGreen();
	afx_msg void OnCustomdrawSliderRed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderBlue(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderGreen(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHITEBALANCEDLG_H__0F0C2EB9_34A1_43E4_BD39_F427AA0BA7CB__INCLUDED_)
