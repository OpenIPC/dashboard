#if !defined(AFX_DLGCFGANALOGALARM_H__05BB7764_6E4A_4A00_B942_870579871D58__INCLUDED_)
#define AFX_DLGCFGANALOGALARM_H__05BB7764_6E4A_4A00_B942_870579871D58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgAnalogAlarm.h : header file
//

typedef struct
{
	EM_SENSE_METHOD			emSenseMethod;
	//em_Alarm_SenseMethod	emSenseMethod;
	char*					pszName;
}Demo_AnalogSenseMethod;

const Demo_AnalogSenseMethod stuDemoAnalogSenseMethod[] = 
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
    {EM_SENSE_MILEAGE, "Mileage"},
	{EM_SENSE_OTHER, "OtherSensor"},
};

typedef struct
{
	int		nMode;
	char*	szMode;
}Demo_Mode;

const Demo_Mode const stuDemoMode[] = {
	{1111, {"1111"}}, {1110, {"1110"}}, {1100, {"1100"}}, {1000, {"1000"}}, {0000, {"0000"}},
};
/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAnalogAlarm dialog

class CDlgCfgAnalogAlarm : public CDialog
{
// Construction
public:
	CDlgCfgAnalogAlarm(CWnd* pParent = NULL, LLONG hLoginID = NULL, int nChannelNum = 16);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgAnalogAlarm)
	enum { IDD = IDD_DLG_CFG_ANALOGALARM };
	CButton	m_chkLevel2;
	CComboBox	m_cmbSensorType;
	CComboBox	m_cmbMode;
	CEdit	m_edtName;
	CComboBox	m_cmbSensorMethod;
	CComboBox	m_cmbChn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgAnalogAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgAnalogAlarm)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgAnalogalarmBtnEventhandler();
	afx_msg void OnCfgAnalogalarmBtnGet();
	afx_msg void OnCfgAnalogalarmBtnSet();
	afx_msg void OnSelchangeCfgAnalogalarmCmbChn();
	afx_msg void OnCfgAlarmChkLevel2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg(BOOL bShow = TRUE);
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	void DlgToStu();
	void StuToDlg();

	BYTE ModeToByte(int nCurSel);
	int	 ModeToInt(BYTE byMode);

private:
	LLONG					m_hLoginID;
	int						m_nChannelNum;
	CFG_ANALOGALARM_INFO	m_stuInfo;
	NET_SENSOR_LIST		m_stuSensorList;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGANALOGALARM_H__05BB7764_6E4A_4A00_B942_870579871D58__INCLUDED_)
