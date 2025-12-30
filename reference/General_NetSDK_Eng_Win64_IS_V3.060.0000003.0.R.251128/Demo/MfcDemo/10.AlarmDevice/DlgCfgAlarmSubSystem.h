#if !defined(AFX_DLGCFGALARMSUBSYSTEM_H__318F586B_3068_465A_8D08_3C58FA0726EA__INCLUDED_)
#define AFX_DLGCFGALARMSUBSYSTEM_H__318F586B_3068_465A_8D08_3C58FA0726EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmSubSystem.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSubSystem dialog

class CDlgCfgAlarmSubSystem : public CDialog
{
// Construction
public:
	CDlgCfgAlarmSubSystem(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmSubSystem)
	enum { IDD = IDD_DLG_CFG_ALARM_SUBSYSTEM };
	CButton	m_chkIsPublic;
	CComboBox	m_cmbSeq;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmSubSystem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmSubSystem)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmsubsystemBtnGet();
	afx_msg void OnAlarmsubsystemBtnSet();
	afx_msg void OnAlarmsubsystemBtnZone();
	afx_msg void OnAlarmsubsystemBtnExZone();
	afx_msg void OnAlarmsubsystemBtnPubsubsystem();
	afx_msg void OnSelchangeAlarmsubsystemCmbSeq();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void		InitDlg();
	void		StuToDlg();
	void		DlgToStu();
	BOOL		GetConfig();
	BOOL		SetConfig();

private:
	LLONG						m_lLoginID;
	CFG_ALARM_SUBSYSTEM_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMSUBSYSTEM_H__318F586B_3068_465A_8D08_3C58FA0726EA__INCLUDED_)
