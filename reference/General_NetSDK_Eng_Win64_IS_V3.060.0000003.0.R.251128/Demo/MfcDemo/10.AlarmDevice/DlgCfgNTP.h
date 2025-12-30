#if !defined(AFX_DLGCFGNTP_H__ECF59CA2_20E5_434D_B8F0_0D2509D9E799__INCLUDED_)
#define AFX_DLGCFGNTP_H__ECF59CA2_20E5_434D_B8F0_0D2509D9E799__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgNTP.h : header file
//

typedef struct
{
	EM_CFG_TIME_ZONE_TYPE	emTimeZone;
	char*					pszName;
}Demo_TimeZone;

const Demo_TimeZone DemoTimeZone[] = 
{
	{EM_CFG_TIME_ZONE_0, "GMT+00:00"},
	{EM_CFG_TIME_ZONE_1, "GMT+01:00"},
	{EM_CFG_TIME_ZONE_2, "GMT+02:00"},
	{EM_CFG_TIME_ZONE_3, "GMT+03:00"},
	{EM_CFG_TIME_ZONE_4, "GMT+03:30"},
	{EM_CFG_TIME_ZONE_5, "GMT+04:00"},
	{EM_CFG_TIME_ZONE_6, "GMT+04:30"},
	{EM_CFG_TIME_ZONE_7, "GMT+05:00"},
	{EM_CFG_TIME_ZONE_8, "GMT+05:30"},
	{EM_CFG_TIME_ZONE_9, "GMT+05:45"},
	{EM_CFG_TIME_ZONE_10, "GMT+06:00"},
	{EM_CFG_TIME_ZONE_11, "GMT+06:30"},
	{EM_CFG_TIME_ZONE_12, "GMT+07:00"},
	{EM_CFG_TIME_ZONE_13, "GMT+08:00"},
	{EM_CFG_TIME_ZONE_14, "GMT+09:00"},
	{EM_CFG_TIME_ZONE_15, "GMT+09:30"},
	{EM_CFG_TIME_ZONE_16, "GMT+10:00"},
	{EM_CFG_TIME_ZONE_17, "GMT+11:00"},
	{EM_CFG_TIME_ZONE_18, "GMT+12:00"},
	{EM_CFG_TIME_ZONE_19, "GMT+13:00"},
	{EM_CFG_TIME_ZONE_20, "GMT-01:00"},
	{EM_CFG_TIME_ZONE_21, "GMT-02:00"},
	{EM_CFG_TIME_ZONE_22, "GMT-03:00"},
	{EM_CFG_TIME_ZONE_23, "GMT-03:30"},
	{EM_CFG_TIME_ZONE_24, "GMT-04:00"},
	{EM_CFG_TIME_ZONE_25, "GMT-05:00"},
	{EM_CFG_TIME_ZONE_26, "GMT-06:00"},
	{EM_CFG_TIME_ZONE_27, "GMT-07:00"},
	{EM_CFG_TIME_ZONE_28, "GMT-08:00"},
	{EM_CFG_TIME_ZONE_29, "GMT-09:00"},
	{EM_CFG_TIME_ZONE_30, "GMT-10:00"},
	{EM_CFG_TIME_ZONE_31, "GMT-11:00"},
	{EM_CFG_TIME_ZONE_32, "GMT-12:00"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNTP dialog

class CDlgCfgNTP : public CDialog
{
	// Construction
public:
	CDlgCfgNTP(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor
	
	// Dialog Data
	//{{AFX_DATA(CDlgCfgNTP)
	enum { IDD = IDD_DLG_CFG_NTP };
	CComboBox	m_cmbTimeZone;
	CButton	m_chkEnable;
	//}}AFX_DATA
	
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgNTP)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CDlgCfgNTP)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgNtpBtnGet();
	afx_msg void OnCfgNtpBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	BOOL GetConfig();
	BOOL SetConfig();
	
private:
	LLONG			m_lLoginId;
	CFG_NTP_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGNTP_H__ECF59CA2_20E5_434D_B8F0_0D2509D9E799__INCLUDED_)
