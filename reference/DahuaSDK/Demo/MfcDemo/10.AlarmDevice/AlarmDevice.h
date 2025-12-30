// AlarmDevice.h : main header file for the ALARMDEVICE application
//

#if !defined(AFX_CLIENTDEMO_H__98F68F83_D38E_403B_9EEA_0C951855AD74__INCLUDED_)
#define AFX_CLIENTDEMO_H__98F68F83_D38E_403B_9EEA_0C951855AD74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define SDK_API_WAIT			(5000)				// delay time of sdk api in msec

// after modification, demo need re-execute
#define SDK_PARAM				"SDKParam"			// for setting AVNetSDK basic params 
#define DEVICE_PARAM			"DeviceParam"		// for setting device basic params

// definition for dlg ui translation
#define DLG_MAIN				"MainDlg"			//
#define	DLG_ACCESS				"AccessManagerDlg"	//=add to control
#define DLG_ALARMBELL			"AlarmBellDlg"		//=move to control
#define DLG_CAPABILITY			"CapabilityDlg"		//
#define DLG_CFG_ALARM			"CfgAlarmDlg"		//
#define DLG_CFG_ALARMCENTER		"CfgAlarmCenter"	//
#define DLG_CFG_ALARMOUT		"CfgAlarmOut"		//
#define DLG_CFG_ANALOGALARM		"CfgAnalogAlarm"	//
#define DLG_CFG_GUARD			"CfgCommGlobal"		//
#define DLG_CFG_LOCALES			"CfgLocales"		//=add to cfg
#define DLG_CFG_MOBILE			"CfgMobile"			//=add to cfg
#define DLG_CFG_NETWORK			"CfgNetwork"		//=add to cfg
#define DLG_CFG_NTP				"CfgNTP"			//=add to cfg
#define DLG_CFG_PHONENOTIFY		"CfgPhoneNotify"	//=add to cfg
#define DLG_CFG_SERIAL			"CfgSerial"			//=add to cfg
#define DLG_CFG_WIRELESS		"CfgWireless"		//=add to cfg
#define DLG_CFG_PSTNPERSONSERVER	"CfgPSTNPersonServer"	//=add to cfg
#define DLG_CFG_ARMLINK			"CfgArmLink"		//=add to cfg
#define DLG_FILEMANAGER			"FileManagerDlg"	//=control
#define DLG_FILENAME			"FileNameDlg"		//=control
#define DLG_LOG_EX				"LogDlg"			//=?
#define DLG_OPT_ACCESS			"OptAccess"			//=control
#define DLG_OPT_PLAYSOUND		"OptPlaySound"		//=control
#define DLG_STATE_BATTERY		"StateBattery"		//
#define DLG_SUBSCRIBE			"SubscribeDlg"		//
#define DLG_TALKING				"TalkingDlg"		//
#define	DLG_USER				"UserPasswordDlg"	//=control
#define DLG_USERINFO			"UserListDlg"		//=control?
#define DLG_VERSION				"VersionDlg"		//=control
#define DLG_EVENTHANDLER		"EventHandlerSubDlg"//
#define DLG_UPGRADE				"UpgradeDlg"		//
#define DLG_PSTN_ALARM_SERVER	"PSTNAlarmServer"	//
#define DLG_ALARM_CHANNEL_STATE	"getAllChannelsState" //=move to control
#define DLG_CFG_ALARMKEYBOARD	"AlarmKeyboard"		//
#define DLG_ALARMKEYBOARD_COUNT	"AlarmKeyboardGetCount"	//
#define DLG_ALARM_CHASSISINTRUSION "ChassisIntrusion"	//
#define DLG_ALARM_POWERFAULT	"PowerFault"		//
#define DLG_CFG_EXALARMBOX		"ExAlarmBox"
#define	DLG_CFG_EXALARMOUTPUT	"ExAlarmOutput"
#define DLG_CFG_EXALARMINPUT	"ExAlarmInput"
#define DLG_EXALARMCHANNEL		"ExAlarmChannel"
#define DLG_EXALARMOUTHANDLER	"ExAlarmOutHandler"
#define DLG_LOCAL_BYPASS		"LocalBypass"
#define DLG_EXTENDED_BYPASS		"ExtendedBypass"
#define DLG_BYPASS				"Bypass"
#define DLG_DEVICETIME			"DeviceTime"
#define DLG_ACTIVATEDDEFENCE	"ActivatedDefence"
#define DLG_RECOVERCONFIG		"RecoverConfig"
#define DLG_QUERYLOG			"QueryLog"
#define DLG_AUTOMAINTAIN		"AutoMainTain"
#define DLG_SETNETWORKPARAM		"SetNetworkParam"
#define DLG_SETAMRMODE			"SetArmMode"
#define DLG_USERMANAGER			"UserManager"
#define DLG_BYPASSCTL			"BypassCtl"
#define DLG_CFG_URGENCY			"Urgency"
#define DLG_CFG_COMMTRANSFER	"Commtransfer"
#define DLG_CFG_STP				"STP"
#define DLG_CFG_SENSORSANMPLING	"SensorSampling"
#define DLG_ANALOGALARMCHANNELS	"AnalogAlarmChannels"
#define DLG_ANALOGALARM_DATA	"AnalogAlarmData"
#define DLG_CFG_SENSORINFO		"VW_SensorInfo"
#define DLG_LOWRATEWPAN			"LowRateWPAN"
#define DLG_ALARM_CHANNELS		"AlarmChannels"
#define DLG_CFG_ALARM_SUBSYSTEM	"CfgAlarmSubSystem"
#define DLG_ALARMSUBSYSTEM		"AlarmSubSystem"
#define	DLG_TALK_BOARDCAST		"TalkBroadcast"
#define	DLG_ADD_DEVICE			"AddDevice"
#define DLG_CFG_SNAPLINKAGE     "CfgSnapLinkage"
#define	DLG_CFG_VTO				"CfgVTO"
#define	DLG_TALK_CENTER 		"TalkCenter"
#define DLG_ALARMOUTSET         "AlarmOutSet"
#define DLG_CFG_AIRCONDITION    "AirCondition"
#define DLG_AIRCONDITION_OPERATE   "AirConditionOperate"
#define DLG_CFG_NETABORT        "CfgNetAbort"
#define DLG_CFG_IPCONFLICT      "CfgIPConflict"
#define DLG_CFG_MACONFLICT      "CfgMacConflict"
#define DLG_CFG_PSTNBREAKLINE   "CfgPSTNBreakLine"
#define DLG_CFG_NETCOLLECTION	"CfgNetCollection"
#define DLG_CFG_ALARMSLOTBOND	"CfgAlarmSlotBond"
#define DLG_CFG_RCEMERGENCYCALL "CfgRCEmergencyCall"
#define DLG_CFG_ALARMBELL       "CfgAlarmBell"

// TimeSection
typedef enum 
{
	em_WeekDay_Sunday = 0,			// 周日
	em_WeekDay_Monday,				// 周一
	em_WeekDay_Tuesday,				// 周二
	em_WeekDay_Wednesday,			// 周三
	em_WeekDay_Thursday,			// 周四
	em_WeekDay_Friday,				// 周五
	em_WeekDay_Saturday,			// 周六
}em_WeekDay;

typedef struct
{
	em_WeekDay emWeekDay;
	char* pszName;
}Demo_WeekDay;

const Demo_WeekDay stuDemoWeekDay[] = 
{
	{em_WeekDay_Sunday, "Sunday"},
	{em_WeekDay_Monday, "Monday"},
	{em_WeekDay_Tuesday, "Tuesday"},
	{em_WeekDay_Wednesday, "Wednesday"},
	{em_WeekDay_Thursday, "Thursday"},
	{em_WeekDay_Friday, "Friday"},
	{em_WeekDay_Saturday, "Saturday"},
};

//////////////////////////////////////////////////////////////////////////
typedef enum tagEMConfigType
{
	emConfigType_Unknown = -1,
	emConfigType_Alarm = 0,
	emConfigType_AlarmChassisIntrusion,
	emConfigType_AlarmExAlarmBox,
	emConfigType_AlarmExAlarmInput,
	emConfigType_AlarmExAlarmOutput,
	emConfigType_AlarmKeyboard,
	emConfigType_AlarmOut,
	emConfigType_AlarmSubSystem,
	emConfigType_AlarmPowerFault,
	emConfigType_AnalogAlarm,
	emConfigType_AutoMainTain,
	emConfigType_BatteryLowPower,
	emConfigType_CommGlobal,
	emConfigType_Locales,
	emConfigType_Mobile,
	emConfigType_AlarmCenter,
	emConfigType_Network,
	emConfigType_NTP,
	emConfigType_PhoneEventNotify,
	emConfigType_PSTN_Alarm_Server,
	emConfigType_SensorSampling,
	emConfigType_Serial,
	emConfigType_SetNetworkParam,
    emConfigType_SnapLinkage,
	emConfigType_STP,
	emConfigType_Urgency,
	emConfigType_VW_SensorInfo,
	emConfigType_WireLess,	
// 	emConfigType_VTO, 
	emConfigType_AirCondition,	
    emConfigType_NetAbort,
    emConfigType_IPConflict,
    emConfigType_MacConflict,
    emConfigType_PSTNBreakLine,
	emConfigType_NetCollection,
	emConfigType_AlarmSlotBond,
    emConfigType_RCEmergencyCall,
    emConfigType_AlarmBell,
    emConfigType_PSTNPersonServer,
    emConfigType_ArmLink,
};

typedef struct tagConfigType
{
	tagEMConfigType		emType;
	const char*			szName;
}AV_ConfigType;

const AV_ConfigType stuConfigType[] = 
{
	//{emConfigType_Unknown, "Unknown"},
	{emConfigType_Alarm, "Alarm Settings"},
	{emConfigType_AlarmChassisIntrusion, "AlarmChassisIntrusion"},
	{emConfigType_AlarmExAlarmBox, "AlarmExAlarmBox"},
	{emConfigType_AlarmExAlarmInput, "AlarmExAlarmInput"},
	{emConfigType_AlarmExAlarmOutput, "AlarmExAlarmOutput"},
	{emConfigType_AlarmKeyboard, "AlarmKeyboard"},
	{emConfigType_AlarmOut, "AlarmOut Settings"},
	{emConfigType_AlarmSubSystem, "AlarmSubSystem"},
	{emConfigType_AlarmPowerFault, "AlarmPowerFault"},
	{emConfigType_AnalogAlarm, "AnalogAlarm"},
	{emConfigType_AutoMainTain, "AutoMainTain"},
	{emConfigType_BatteryLowPower, "BatteryLowPower"},
	{emConfigType_CommGlobal, "CommGlobal Settings"},
	{emConfigType_Locales, "Locales"},
	{emConfigType_Mobile, "Mobile"},
	{emConfigType_AlarmCenter, "NetServer Settings"},
	{emConfigType_Network, "Network Settings"},
	{emConfigType_NTP, "NTP"},
	{emConfigType_PhoneEventNotify, "PhoneNotify"},
	{emConfigType_PSTN_Alarm_Server, "PSTNAlarmServer"},
	{emConfigType_SensorSampling, "SensorSampling"},
	{emConfigType_Serial, "Serial Settings"},
	{emConfigType_SetNetworkParam, "SetNetworkParam"},
    {emConfigType_SnapLinkage, "SnapLinkage"},
	{emConfigType_STP, "STP"},
	{emConfigType_Urgency, "Urgency(Emergency)"},
	{emConfigType_VW_SensorInfo, "VW_SensorInfo"},
	{emConfigType_WireLess, "WireLess Settings"},
// 	{emConfigType_VTO, "VTO Settings"},
	{emConfigType_AirCondition, "AirCondition"},
    {emConfigType_NetAbort, "NetAbort"},
    {emConfigType_IPConflict, "IPConflict"},
    {emConfigType_MacConflict, "MacConflict"},
    {emConfigType_PSTNBreakLine, "PSTNBreakLine"},
    {emConfigType_NetCollection, "NetCollection"},
    {emConfigType_AlarmSlotBond, "AlarmSlotBond"},
    {emConfigType_RCEmergencyCall, "RCEmergencyCall"},
    {emConfigType_AlarmBell, "AlarmBell"},
    {emConfigType_PSTNPersonServer, "PSTNPersonServer"},
    {emConfigType_ArmLink, "ArmLink"},
};

//////////////////////////////////////////////////////////////////////////
typedef enum tagEMControlQueryType
{
	//emControlType_Unkonwn = -1,
	emControlType_AccessManager = 0,
	emControlType_ActivatedDefenceArea,
	emControlType_AlarmBell,//
	emControlType_AlarmChannels,
	emControlType_AlarmChannelState,
    emControlType_AlarmOutSet,
	emControlType_AlarmSubSystem,
	//emControlType_AlarmKeyboard,
	//emControlType_AttachCfgChange,
	emControlType_AnalogAlarmChannels,
	emControlType_AnalogAlarmData,
	emControlType_AudioPlay,//
	emControlType_SetBypass,
	emControlType_DeviceTime,
	emControlType_AlarmExAlarmChannel,
	emControlType_FileManager,
	emControlType_FileName,//
	emControlType_Log,//
	emControlType_LowRateWPAN,
	emControlType_ModifyPasswd,
	emControlType_OpenAccess,
	emControlType_PowerState,
	emControlType_RecoverConfig,
	emControlType_SetArmMode,
	emControlType_Version,//
	emControlType_UserList,//
	emControlType_UserManager,
	emControlType_Upgrade,//
	//emControlType_QueryLog,
//     emControlType_TalkCenter, 
    emControlType_AirConditonOperate,
}emControlQueryType;

typedef struct tagControlQueryType
{
	emControlQueryType	emType;
	const char*			szName;
}ControlQueryType;


const ControlQueryType stuControQueryType[] = 
{
	//{emConfigType_Unknown, "Unknown"},
	{emControlType_AccessManager, "AccessManager"},
	{emControlType_ActivatedDefenceArea, "ActivatedDefenceArea"},
	{emControlType_AlarmBell, "AlarmBell"},
	{emControlType_AlarmChannels, "AlarmChannels"},
    {emControlType_AlarmChannelState, "AlarmChannelState"},
	{emControlType_AlarmOutSet, "AlarmOutSet"},
	{emControlType_AlarmSubSystem, "AlarmSubSystem"},
	//{emControlType_AlarmKeyboard, "AlarmKeyboard"}, =>in Capability Window
	//{emControlType_AttachCfgChange, "AttachCfgChange"},
	{emControlType_AnalogAlarmChannels, "AnalogAlarmChannels"},
	{emControlType_AnalogAlarmData, "AnalogAlarmData"},
	{emControlType_AudioPlay, "AudioPlay"},
	{emControlType_SetBypass, "Bypass"},
	{emControlType_DeviceTime, "DeviceTime"},
	{emControlType_AlarmExAlarmChannel, "ExAlarmChannel"},	
	{emControlType_FileManager, "FileManager"},
	{emControlType_FileName, "FileName"},
	{emControlType_Log, "Log"},//=>go to LogDemo in detail
	{emControlType_LowRateWPAN, "LowRateWPAN"},
	{emControlType_ModifyPasswd, "ModifyPsw"},
	{emControlType_OpenAccess, "OpenAccess"},
	{emControlType_PowerState, "PowerState"},
	{emControlType_RecoverConfig, "RecoverConfig"},
	{emControlType_SetArmMode, "SetArmMode"},
	{emControlType_Version, "Version"},
	{emControlType_UserList, "UserList"},
    {emControlType_UserManager, "UserManager"},
    {emControlType_Upgrade, "Upgrade"},
// 	{emControlType_TalkCenter, "TalkCenter"},
	//{emControlType_QueryLog, "QueryLog"},
    {emControlType_AirConditonOperate, "AirConditionOperate"},
};


//////////////////////////////////////////////////////////////////////////

// 防区类型
typedef struct
{
	EM_CFG_DEFENCEAREATYPE emType;
	char* pszName;
}Demo_AreaType;

const Demo_AreaType stuDemoAreaType[] = 
{
	{EM_CFG_DefenceAreaType_Unknown, "Unknown"},
	{EM_CFG_DefenceAreaType_InTime, "Intime"},
	{EM_CFG_DefenceAreaType_Delay, "Delay"},
	{EM_CFG_DefenceAreaType_FullDay, "Fullday"},
    {EM_CFG_DefenceAreaType_Fire, "Fire"},
    {EM_CFG_DefenceAreaType_FullDaySound, "FullDaySound"},
    {EM_CFG_DefenceAreaType_FullDaySlient, "FullDaySlient"},
    {EM_CFG_DefenceAreaType_Entrance1, "Entrance1"},
    {EM_CFG_DefenceAreaType_Entrance2, "Entrance2"},
    {EM_CFG_DefenceAreaType_InSide, "InSide"},
    {EM_CFG_DefenceAreaType_OutSide, "OutSide"},
};

// 门禁操作
typedef struct 
{
	EM_CFG_ACCESSCONTROLTYPE emType;
	char* pszName;
}Demo_AccessControlType;

const Demo_AccessControlType stuAccessControlType[] = 
{
	{EM_CFG_ACCESSCONTROLTYPE_NULL, "Null"},
	{EM_CFG_ACCESSCONTROLTYPE_AUTO, "Auto"},
	{EM_CFG_ACCESSCONTROLTYPE_OPEN, "Open"},
	{EM_CFG_ACCESSCONTROLTYPE_CLOSE, "Close"},
	{EM_CFG_ACCESSCONTROLTYPE_OPENALWAYS, "OpenAlways"},
	{EM_CFG_ACCESSCONTROLTYPE_CLOSEALWAYS, "CloseAlways"}
};

// For Loading PlaySDK dynamically
extern CPlayAPI s_PlayAPI;

// mode
typedef struct
{
    EM_CFG_AIRCONDITION_MODE	emMode;
    char*		                pszName;
}Demo_AirConditon_Mode;

const Demo_AirConditon_Mode stuDemoAirConditionMode[] = 
{
    {EM_CFG_AIRCONDITION_MODE_AUTO, "Auto"},
    {EM_CFG_AIRCONDITION_MODE_HOT, "Hot"},
    {EM_CFG_AIRCONDITION_MODE_COLD, "Cold"},
    {EM_CFG_AIRCONDITION_MODE_WET, "Wet"},
    {EM_CFG_AIRCONDITION_MODE_WIND, "Wind"},
};

// WindMode
typedef struct
{
    EM_CFG_AIRCONDITION_WINDMODE	emWindMode;
    char*					        pszName;
}Demo_AirConditon_WindMode;

const Demo_AirConditon_WindMode stuDemoWindMode[] = 
{
    {EM_CFG_AIRCONDITION_WINDMODE_STOP, "Stop"},
    {EM_CFG_AIRCONDITION_WINDMODE_AUTO, "Auto"},
    {EM_CFG_AIRCONDITION_WINDMODE_HIGH, "High"},
    {EM_CFG_AIRCONDITION_WINDMODE_MIDDLE, "Middle"},
    {EM_CFG_AIRCONDITION_WINDMODE_LOW, "Low"},
};

/////////////////////////////////////////////////////////////////////////////
// CAlarmDeviceApp:
// See AlarmDevice.cpp for the implementation of this class
//

class CAlarmDeviceApp : public CWinApp
{
public:
	CAlarmDeviceApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClientDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// void g_SetWndStaticText(CWnd * pWnd);
// CString ConvertString(CString strText);

void g_SetWndStaticText(CWnd * pWnd, const char* pszSeg = NULL);
//--------------------------------------------------------------------------------
// convert GUI text
CString ConvertString(const CString& strText, const char* pszSeg = NULL);

// convert enum to str
void SenseTypeToStr(NET_SENSE_METHOD nSenseType, CString& szStr);

// convert status of analog alarm data to str
CString AnalogAlarmDataStatusToStr(int nStatus);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTDEMO_H__98F68F83_D38E_403B_9EEA_0C951855AD74__INCLUDED_)
