#if !defined(AFX_DLGCFGAUTOMAINTAIN_H__EBBA0A40_0946_4624_A2EB_8A5E5AB9B3CA__INCLUDED_)
#define AFX_DLGCFGAUTOMAINTAIN_H__EBBA0A40_0946_4624_A2EB_8A5E5AB9B3CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAutoMaintain.h : header file
//
enum emAutoMainTainDay
{
	emAutoMainTainDay_Never,
	emAutoMainTainDay_Sunday,
	emAutoMainTainDay_Monday,
	emAutoMainTainDay_Tuesday,
	emAutoMainTainDay_Wednesday,
	emAutoMainTainDay_Thursday,
	emAutoMainTainDay_Friday,
	emAutoMainTainDay_Saturday,
	emAutoMainTainDay_EveryDay,
};

typedef struct
{
	emAutoMainTainDay		emDay;
	char*					pszName;
}Demo_DayType;

const Demo_DayType stuAutoMianTainDay[] = {
	{emAutoMainTainDay_Never, "Never"},
	{emAutoMainTainDay_Never, "Sunday"},
	{emAutoMainTainDay_Never, "Monday"},
	{emAutoMainTainDay_Never, "Tuesday"},
	{emAutoMainTainDay_Never, "Wednesday"},
	{emAutoMainTainDay_Never, "Thursday"},
	{emAutoMainTainDay_Never, "Friday"},
	{emAutoMainTainDay_Never, "Saturday"},
	{emAutoMainTainDay_Never, "EveryDay"},
};

#define WM_ALARM_INFO	(WM_USER+199)

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAutoMaintain dialog

class CDlgCfgAutoMaintain : public CDialog
{
// Construction
public:
	CDlgCfgAutoMaintain(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAutoMaintain)
	enum { IDD = IDD_DLG_CFG_AUTOMAINTAIN };
	CComboBox	m_cmbStartUpDay;
	CComboBox	m_cmbShutDownDay;
	CComboBox	m_cmbRebootDay;
	CButton	m_ckRebootEnable;
	COleDateTime	m_timeRebootTime;
	COleDateTime	m_timeShutDownTime;
	COleDateTime	m_timeShutUpTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAutoMaintain)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAutoMaintain)
	virtual BOOL OnInitDialog();
	afx_msg void OnAutomaintainBtnGet();
	afx_msg void OnAutomaintainBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void StuToDlg();
	void DlgToStu();

private:
	LLONG					m_lLoginId;
	AV_CFG_AutoMaintain		m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGAUTOMAINTAIN_H__EBBA0A40_0946_4624_A2EB_8A5E5AB9B3CA__INCLUDED_)
