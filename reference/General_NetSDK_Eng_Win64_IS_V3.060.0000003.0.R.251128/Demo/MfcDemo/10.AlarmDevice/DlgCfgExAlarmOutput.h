#if !defined(AFX_DLGCFGEXALARMOUTPUT_H__1EA8B562_A92D_41FC_BA2F_3879781A30F4__INCLUDED_)
#define AFX_DLGCFGEXALARMOUTPUT_H__1EA8B562_A92D_41FC_BA2F_3879781A30F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgExAlarmOutput.h : header file
//
enum em_ExOutput_Mode
{
	em_ExOutput_Mode_AutoAlarm = 0,		// 自动报警
	em_ExOutput_Mode_ForceAlarm,		// 强制报警
	em_ExOutput_Mode_CloseAlarm,		// 关闭报警
};

typedef struct
{
	em_ExOutput_Mode	emOutputMode;
	char*				pszName;
}Demo_ExOutput_Mode;

const Demo_ExOutput_Mode DemoExOutputMode[] = 
{
	{em_ExOutput_Mode_AutoAlarm, "AutoAlarm"},
	{em_ExOutput_Mode_ForceAlarm, "ForceAlarm"},
	{em_ExOutput_Mode_CloseAlarm, "CloseAlarm"}
};
/////////////////////////////////////////////////////////////////////////////
// CDlgCfgExAlarmOutput dialog

class CDlgCfgExAlarmOutput : public CDialog
{
// Construction
public:
	CDlgCfgExAlarmOutput(CWnd* pParent = NULL, LLONG lLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgExAlarmOutput)
	enum { IDD = IDD_DLG_CFG_ALARMEXALARMOUTPUT };
	CComboBox	m_cbChannel;
	CComboBox	m_cmbMode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgExAlarmOutput)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgExAlarmOutput)
	afx_msg void OnExalarmoutputBtnGet();
	afx_msg void OnExalarmoutputBtnSet();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeExalarmoutputComboChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	BOOL GetAlarmOutConfig();
	BOOL SetAlarmOutConfig();

	LLONG			m_lLoginId;
	CFG_EXALARMOUTPUT_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGEXALARMOUTPUT_H__1EA8B562_A92D_41FC_BA2F_3879781A30F4__INCLUDED_)
