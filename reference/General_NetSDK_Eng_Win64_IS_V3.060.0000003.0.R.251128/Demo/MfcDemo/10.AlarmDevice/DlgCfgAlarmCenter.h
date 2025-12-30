#if !defined(AFX_DLGCFGALARMCENTER_H__77AC2BD0_D676_4931_ABDE_267271E44376__INCLUDED_)
#define AFX_DLGCFGALARMCENTER_H__77AC2BD0_D676_4931_ABDE_267271E44376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmCenter.h : header file
//

typedef struct tagReportWeekDay 
{
	emCFG_REPORTWEEKDAY	emWeekDay;
	const char*			szName;
}Demo_ReportWeekDay;

const Demo_ReportWeekDay stuDemoReportWeekDay[] = {
	{emReportWeekDay_DoNotReport, "DoNotReport"},
	{emReportWeekDay_Sunday, "Sunday"},
	{emReportWeekDay_Monday, "Monday"},
	{emReportWeekDay_Tuesday, "Tuesday"},
	{emReportWeekDay_Wednesday, "Wednesday"},
	{emReportWeekDay_Thursday, "Thursday"},
	{emReportWeekDay_Friday, "Friday"},
	{emReportWeekDay_Saturday, "Saturday"},
	{emReportWeekDay_Everyday, "Everyday"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmCenter dialog
#define MAX_NUM 8

class CDlgCfgAlarmCenter : public CDialog
{
// Construction
public:
	CDlgCfgAlarmCenter(CWnd* pParent = NULL, LLONG hLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmCenter)
	enum { IDD = IDD_DLG_CFG_ALARMCENTER };
	CIPAddressCtrl	m_ctrlIP;
	CComboBox	m_cmbSecond;
	CComboBox	m_cmbMin;
	CComboBox	m_cmbHour;
	CComboBox	m_cbUpday;
	CButton	m_ckUpEnable;
	//}}AFX_DATA
	
public:
	void	InitDlg();
	BOOL	getInfo();
	void	showInfo();
	BOOL	SetConfigToDevice();
	BOOL	GetConfigFromDevice();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmCenter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmCenter)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmcenterButtonGetA3();
	afx_msg void OnAlarmcenterButtonSetA3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG					m_hLogin;
	CFG_ALARMCENTER_INFO	m_stuAlarmServerInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMCENTER_H__77AC2BD0_D676_4931_ABDE_267271E44376__INCLUDED_)
