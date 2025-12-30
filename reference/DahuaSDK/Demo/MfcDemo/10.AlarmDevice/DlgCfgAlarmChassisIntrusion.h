#if !defined(AFX_DLGCFGALARMCHASSISINTRUSION_H__4A40CA48_348F_4D0F_B57C_F6827A7A909A__INCLUDED_)
#define AFX_DLGCFGALARMCHASSISINTRUSION_H__4A40CA48_348F_4D0F_B57C_F6827A7A909A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmChassisIntrusion.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmChassisIntrusion dialog

class CDlgCfgAlarmChassisIntrusion : public CDialog
{
// Construction
public:
	BOOL getInfo();
	BOOL showInfo();
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	CDlgCfgAlarmChassisIntrusion(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmChassisIntrusion)
	enum { IDD = IDD_DLG_CFG_ALARM_CHASSISINTRUSION };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmChassisIntrusion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmChassisIntrusion)
	afx_msg void OnChassisintrusionButtonGet();
	afx_msg void OnChassisintrusionButtonSet();
	afx_msg void OnChassisintrusionButtonEventhandler();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG			m_lLoginId;
	CFG_CHASSISINTRUSION_INFO	m_stuChassisIntrusion;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMCHASSISINTRUSION_H__4A40CA48_348F_4D0F_B57C_F6827A7A909A__INCLUDED_)
