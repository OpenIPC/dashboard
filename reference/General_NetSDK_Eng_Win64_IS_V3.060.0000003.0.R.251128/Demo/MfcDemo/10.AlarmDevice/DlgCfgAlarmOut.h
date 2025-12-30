#if !defined(AFX_DLGCFGALARMOUT_H__6EC1B2D3_942B_48BB_8E21_8D2CE092C5AF__INCLUDED_)
#define AFX_DLGCFGALARMOUT_H__6EC1B2D3_942B_48BB_8E21_8D2CE092C5AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarmOut.h : header file
//

enum em_Output_Mode
{
	em_Output_Mode_AutoAlarm = 0,	// 自动报警
	em_Output_Mode_ForceAlarm,		// 强制报警
	em_Output_Mode_CloseAlarm,		// 关闭报警
};

typedef struct
{
	em_Output_Mode	emOutputMode;
	char*			pszName;
}Demo_Output_Mode;

const Demo_Output_Mode DemoOutputMode[] = 
{
	{em_Output_Mode_AutoAlarm, "AutoAlarm"},
	{em_Output_Mode_ForceAlarm, "ForceAlarm"},
	{em_Output_Mode_CloseAlarm, "CloseAlarm"}
};

/////////////////////////////////////////////////////////////////////////////
// DlgCfgAlarmOut dialog

class DlgCfgAlarmOut : public CDialog
{
// Construction
public:
	DlgCfgAlarmOut(CWnd* pParent = NULL, LLONG hLoginId = NULL, UINT32 uiAlarmOut = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgCfgAlarmOut)
	enum { IDD = IDD_DLG_CFG_ALARMOUT };
	CComboBox	m_cbOutMode;
	CComboBox	m_cbChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgCfgAlarmOut)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgCfgAlarmOut)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmoutGet();
	afx_msg void OnAlarmoutSet();
	afx_msg void OnSelchangeAlarmoutCmbChn();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	BOOL GetAlarmOutConfig();
	BOOL SetAlarmOutConfig();

private:
	LLONG				m_hLoginId;
	UINT32				m_uiAlarmOut;
	CFG_ALARMOUT_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARMOUT_H__6EC1B2D3_942B_48BB_8E21_8D2CE092C5AF__INCLUDED_)
