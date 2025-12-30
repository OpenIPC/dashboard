#if !defined(AFX_DLGALARMSUBSYSTEM_H__37CAFB24_45F4_4F2D_AB65_EF2806DB3ABA__INCLUDED_)
#define AFX_DLGALARMSUBSYSTEM_H__37CAFB24_45F4_4F2D_AB65_EF2806DB3ABA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAlarmSubSystem.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmSubSystem dialog

class CDlgAlarmSubSystem : public CDialog
{
// Construction
public:
	CDlgAlarmSubSystem(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAlarmSubSystem)
	enum { IDD = IDD_DLG_ALARMSUBSYSTEM };
	CComboBox	m_cmbChannelId;
	CButton	m_ckActive;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarmSubSystem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarmSubSystem)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmsubsystemBtnGet();
	afx_msg void OnAlarmsubsystemBtnSet();
	afx_msg void OnSelchangeAlarmsubsystemCmbChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LLONG		m_lLoginId;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARMSUBSYSTEM_H__37CAFB24_45F4_4F2D_AB65_EF2806DB3ABA__INCLUDED_)
