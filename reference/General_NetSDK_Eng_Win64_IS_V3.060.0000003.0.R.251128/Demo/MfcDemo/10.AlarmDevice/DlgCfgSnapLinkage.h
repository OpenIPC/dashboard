#if !defined(AFX_DLGCFGSNAPLINKAGE_H__8A3088C5_048B_40CF_84CA_886449AA79F2__INCLUDED_)
#define AFX_DLGCFGSNAPLINKAGE_H__8A3088C5_048B_40CF_84CA_886449AA79F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgSnapLinkage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSnapLinkage dialog

class CDlgCfgSnapLinkage : public CDialog
{
// Construction
public:
	CDlgCfgSnapLinkage(CWnd* pParent = NULL, LLONG lLoginID = NULL, UINT32 uiVideoIn = 0, UINT32 uiAnalogAlarmIn = 72);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgSnapLinkage)
	enum { IDD = IDD_DLG_CFG_SNAPLINKAGE };
	CComboBox	m_cmbChn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgSnapLinkage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgSnapLinkage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCfgSnaplinkageCmbChn();
	afx_msg void OnCfgSnaplinkageBtnChns();
	afx_msg void OnCfgSnaplinkageBtnGet();
	afx_msg void OnCfgSnaplinkageBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void        InitDlg();
	BOOL        SetConfigToDevice();
    BOOL        GetConfigFromDevice();
    void        DlgToStu();
	void        StuToDlg();

private:
    LLONG       m_lLoginID;
    UINT32      m_uiVideoIn;
    UINT32      m_uiAnalogAlarmIn;
    CFG_SNAPLINKAGE_INFO    m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGSNAPLINKAGE_H__8A3088C5_048B_40CF_84CA_886449AA79F2__INCLUDED_)
