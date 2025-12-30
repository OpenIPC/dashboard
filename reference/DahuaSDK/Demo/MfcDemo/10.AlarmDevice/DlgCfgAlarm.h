#if !defined(AFX_DLGCFGALARM_H__C6DC763E_1C4E_4BE4_9F4F_74B372F7635F__INCLUDED_)
#define AFX_DLGCFGALARM_H__C6DC763E_1C4E_4BE4_9F4F_74B372F7635F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAlarm.h : header file
//

// enum em_Alarm_SenseMethod
// {
// 	em_Alarm_SenseMethod_OtherSensor = -1,
// 	em_Alarm_SenseMethod_DoorMagnetism,
// 	em_Alarm_SenseMethod_PassiveInfrared,
// 	em_Alarm_SenseMethod_GasSensor,
// 	em_Alarm_SenseMethod_SmokingSensor,
// 	em_Alarm_SenseMethod_WaterSensor,
// 	em_Alarm_SenseMethod_ActiveInfrared,
// };

typedef struct
{
	EM_SENSE_METHOD			emSenseMethod;
	//em_Alarm_SenseMethod	emSenseMethod;
	char*					pszName;
}Demo_SenseMethod;

const Demo_SenseMethod stuDemoSenseMethod[] = 
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
	{EM_SENSE_GASPRESSURE, "GasPressure"},
	{EM_SENSE_GASCONCENTRATION, "GasConcentration"},
	{EM_SENSE_GASFLOW, "GasFlow"},
	{EM_SENSE_OIL, "Oil"},
	{EM_SENSE_MILEAGE, "MileageSensor"},
	{EM_SENSE_OTHER, "OtherSensor"},
};
/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarm dialog

class CDlgCfgAlarm : public CDialog
{
// Construction
public:
	CDlgCfgAlarm(CWnd* pParent = NULL, LLONG hLoginID = NULL, UINT32 uiAlarmIn = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAlarm)
	enum { IDD = IDD_DLG_CFG_ALARM };
	CButton	m_chkLevel2;
	CEdit	m_edtName;
	CEdit	m_edtEnableDelay;
	CEdit	m_edtDisableDelay;
	CComboBox	m_cmbSensorType;
	CComboBox	m_cmbSensorMethod;
	CComboBox	m_cmbChn;
	CComboBox	m_cmbAreaType;
	CButton	m_chkByPass;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAlarm)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgAlarmBtnEventhandler();
	afx_msg void OnCfgAlarmBtnGet();
	afx_msg void OnCfgAlarmBtnSet();
	afx_msg void OnSelchangeCfgAlarmCmbChn();
	afx_msg void OnCfgAlarmChkLevel2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg(BOOL bShow = TRUE);
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	void DlgToStu();
	void StuToDlg();

private:
	LLONG					m_hLoginID;
	CFG_ALARMIN_INFO		m_stuInfo;
	BOOL					m_bNeedResetWaittime;
	// 本地报警输入通道数
	UINT32					m_uiAlarmIn;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGALARM_H__C6DC763E_1C4E_4BE4_9F4F_74B372F7635F__INCLUDED_)
