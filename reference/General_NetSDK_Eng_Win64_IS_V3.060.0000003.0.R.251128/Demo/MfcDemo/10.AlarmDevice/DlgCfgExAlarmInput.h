#if !defined(AFX_DLGCFGEXALARMINPUT_H__8B1F23C4_C3C7_4E5F_A450_A86592A33833__INCLUDED_)
#define AFX_DLGCFGEXALARMINPUT_H__8B1F23C4_C3C7_4E5F_A450_A86592A33833__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgExAlarmInput.h : header file
//

typedef struct
{
	EM_SENSE_METHOD			emSenseMethod;
	char*					pszName;
}Demo_ExAlarm_SenseMethod;

const Demo_ExAlarm_SenseMethod stuDemoExAlarmSenseMethod[] = 
{
	{EM_SENSE_DOOR, "DoorMagnetism"},
	{EM_SENSE_PASSIVEINFRA, "PassiveInfrared"},
	{EM_SENSE_GAS, "GasSensor"},
	{EM_SENSE_SMOKING, "SmokingSensor"},
	{EM_SENSE_WATER, "WaterSensor"},
	{EM_SENSE_ACTIVEFRA, "ActiveInfrared"},
	{EM_SENSE_GLASS, "GlassSensor"},
	{EM_SENSE_EMERGENCYSWITCH, "EmergencySwitch"},
	{EM_SENSE_SHOCK, "ShockSensor"},
	{EM_SENSE_DOUBLEMETHOD, "DoubleMethod"},
	{EM_SENSE_THREEMETHOD, "ThreeMethod"},
	{EM_SENSE_TEMP, "TempSensor"},
	{EM_SENSE_HUMIDITY, "HumiditySensor"},
    {EM_SENSE_WIND, "WindSensor"},
	{EM_SENSE_CALLBUTTON, "CallButton"},
	{EM_SENSE_OTHER, "OtherSensor"},
};
/////////////////////////////////////////////////////////////////////////////
// CDlgCfgExAlarmInput dialog

class CDlgCfgExAlarmInput : public CDialog
{
// Construction
public:
	CDlgCfgExAlarmInput(CWnd* pParent = NULL, LLONG lLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgExAlarmInput)
	enum { IDD = IDD_DLG_CFG_ALARMEXALARMINPUT };
		// NOTE: the ClassWizard will add data members here
	CEdit	m_edtName;
	CComboBox	m_cmbSensorMethod;
	CComboBox	m_cmbChn;
	CButton	m_chkByPass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgExAlarmInput)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgExAlarmInput)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgAlarmBtnEventhandler();
	afx_msg void OnCfgAlarmBtnGet();
	afx_msg void OnCfgAlarmBtnSet();
	afx_msg void OnSelchangeCfgAlarmCmbChn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg(BOOL bShow = TRUE);
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	void DlgToStu();
	void StuToDlg();

private:
	LLONG					m_lLoginID;
	CFG_EXALARMINPUT_INFO	m_stuInfo;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGEXALARMINPUT_H__8B1F23C4_C3C7_4E5F_A450_A86592A33833__INCLUDED_)
