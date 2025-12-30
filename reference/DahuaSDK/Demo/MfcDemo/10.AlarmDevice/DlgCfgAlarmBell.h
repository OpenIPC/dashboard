#if !defined(AFX_DLGCFGALARMBELL_H__EACC982B_7078_4764_AC60_06055CAF34BC__INCLUDED_)
#define AFX_DLGCFGALARMBELL_H__EACC982B_7078_4764_AC60_06055CAF34BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmBell.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmBell dialog

class CDlgCfgAlarmBell : public CDialog
{
// Construction
public:
	CDlgCfgAlarmBell(CWnd* pParent = NULL, LLONG lLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarmBell)
	enum { IDD = IDD_DLG_CFG_ALARMBELL };
	CComboBox	m_cmbChn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarmBell)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarmBell)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgAlarmbellGet();
	afx_msg void OnCfgAlarmbellSet();
	afx_msg void OnSelchangeCfgAlarmbellCmbChn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
        
private:
    void InitDlg();
    BOOL SetConfigToDevice();
    BOOL GetConfigFromDevice();
    void DlgToStu();
	void StuToDlg();

private:
    LLONG               m_lLoginID;
    CFG_ALARMBELL_INFO  m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMBELL_H__EACC982B_7078_4764_AC60_06055CAF34BC__INCLUDED_)
