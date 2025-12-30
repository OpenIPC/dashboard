#if !defined(AFX_DLGCFGALARMSLOTBOND_H__5D1F7844_C956_49A1_A9F3_95CA921688D0__INCLUDED_)
#define AFX_DLGCFGALARMSLOTBOND_H__5D1F7844_C956_49A1_A9F3_95CA921688D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmSlotBond.h : header file
//

typedef struct tagDemoInterfaceType 
{
	EM_CFG_INTERFACE_TYPE	emType;
	const char*				szInfo;
}DemoInterfaceType;

const DemoInterfaceType stuDemoInterfaceType[] = {
	{EM_CFG_INTERFACE_TYPE_UNKNOWN, "Unknown"},
	{EM_CFG_INTERFACE_TYPE_COMM, "Comm"},
	{EM_CFG_INTERFACE_TYPE_NETCOLLECTION, "NetCollection"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSlotBond dialog

class CDlgCfgAlarmSlotBond : public CDialog
{
// Construction
public:
	CDlgCfgAlarmSlotBond(CWnd* pParent = NULL, LLONG lLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmSlotBond)
	enum { IDD = IDD_DLG_CFG_ALARMSLOTBOND };
	CComboBox	m_cmbType;
	CComboBox	m_cmbIndex;
	CComboBox	m_cmbChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmSlotBond)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmSlotBond)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmslotbondBtnGet();
	afx_msg void OnAlarmslotbondBtnSet();
	afx_msg void OnSelchangeAlarmslotbondCmbChn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void					InitDlg();
    BOOL					getInfo();
    BOOL					showInfo();
    BOOL					SetConfigToDevice();
    BOOL					GetConfigFromDevice();
	
private:
	LLONG					m_lLoginID;
	CFG_ALARM_SLOT_BOND_INFO m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMSLOTBOND_H__5D1F7844_C956_49A1_A9F3_95CA921688D0__INCLUDED_)
