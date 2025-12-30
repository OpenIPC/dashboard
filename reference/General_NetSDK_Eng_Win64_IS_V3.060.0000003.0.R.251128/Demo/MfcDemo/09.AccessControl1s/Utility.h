#if !defined(_UTILITY_TT_)
#define _UTILITY_TT_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef NEW
#define NEW new(std::nothrow)
#endif

#include "Device.h"
#include <string>

#define	PC_DATA_STORE_FOLDER		"PCStore\\"
#define CFG_CFG_FILE_NAME			"Config.ini"
#define CFG_SECTION_CFG				"CFG"
#define CFG_KEY_DATA_PATH			"DATA_PATH"


typedef enum tagem_WeekDay
{
	em_WeekDay_Sunday = 0,			
	em_WeekDay_Monday,				
	em_WeekDay_Tuesday,				
	em_WeekDay_Wednesday,			
	em_WeekDay_Thursday,			
	em_WeekDay_Friday,				
	em_WeekDay_Saturday,			
}em_WeekDay;

typedef struct tagDemo_WeekDay
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

typedef struct tagOpenMethod 
{
	CFG_DOOR_OPEN_METHOD	emOpenMethod;
	const char*				szName;
}OpenMethod;

const OpenMethod stuDemoOpenMethod[] = 
{
	{CFG_DOOR_OPEN_METHOD_UNKNOWN, "Unknown"},
	{CFG_DOOR_OPEN_METHOD_PWD_ONLY, "PwdOnly"},
	{CFG_DOOR_OPEN_METHOD_CARD, "Card"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINTONLY, "FingerPrintOnly"},
	{CFG_DOOR_OPEN_METHOD_CARD_FIRST, "CardFirst"},
	{CFG_DOOR_OPEN_METHOD_PWD_FIRST, "PwdFirst"},
	{CFG_DOOR_OPEN_METHOD_PWD_OR_CARD, "PwdOrCard"},
	{CFG_DOOR_OPEN_METHOD_PWD_OR_CARD_OR_FINGERPRINT, "PwdOrCardOrFingerPrint"},
	{CFG_DOOR_OPEN_METHOD_PWD_AND_CARD_AND_FINGERPINT, "Pwd+Card+FingerPrint"},
	{CFG_DOOR_OPEN_METHOD_PWD_AND_FINGERPRINT, "Pwd+FingerPrint"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_FINGERPRINT, "CardAndFingerPrint"},
	{CFG_DOOR_OPEN_METHOD_MULTI_PERSON, "MultiPerson"},
	{CFG_DOOR_OPEN_METHOD_SECTION, "TimeSection"},
};

typedef struct tagAccessState 
{
	CFG_ACCESS_STATE		emAccessStart;
	const char*				szName;
}AccessState;

const AccessState stuDemoAccessState[] = 
{
	{ACCESS_STATE_NORMAL, "Normal"},
	{ACCESS_STATE_CLOSEALWAYS, "CloseAlways"},
	{ACCESS_STATE_OPENALWAYS, "OpenAlways"},
};

typedef enum tagEm_RecordSet_Operate_Type
{
	Em_Operate_Type_Show = 0,		
	Em_Operate_Type_Insert,			
	Em_Operate_Type_Get,			
	Em_Operate_Type_Update,			
	Em_Operate_Type_Remove,			
	Em_Operate_Type_Clear,			
	Em_Operate_Type_InsertEX,		
	Em_Operate_Type_UpdateEX,		
}Em_RecordSet_Operate_Type;

typedef struct tagDemoRecordSetCtlType
{
	Em_RecordSet_Operate_Type	emType;
	const char*					szName;
}DemoRecordSetCtlType;

const DemoRecordSetCtlType stuDemoRecordSetCtlType[] = 
{
	{Em_Operate_Type_Insert,	"Insert"},
	{Em_Operate_Type_Get,		"Get"},
	{Em_Operate_Type_Update,	"Update"},
	{Em_Operate_Type_Remove,	"Remove"},
	{Em_Operate_Type_Clear,		"Clear"},
};

const DemoRecordSetCtlType stuDemoCardSetCtlType[] = 
{
	{Em_Operate_Type_Insert,	"Insert"},
	{Em_Operate_Type_Get,		"Get"},
	{Em_Operate_Type_Update,	"Update"},
	{Em_Operate_Type_Remove,	"Remove"},
	{Em_Operate_Type_Clear,		"Clear"},
	{Em_Operate_Type_InsertEX,	"InsertEX"},
	{Em_Operate_Type_UpdateEX,	"UpdateEX"},
};


typedef struct tagOpenDoorGroupSetCtlType
{
	EM_CFG_OPEN_DOOR_GROUP_METHOD	emType;
	const char*						szName;
}OpenDoorGroupSetCtlType;

const OpenDoorGroupSetCtlType stuOpenDoorGroupSetCtlType[] = 
{
	{EM_CFG_OPEN_DOOR_GROUP_METHOD_UNKNOWN,		"Unknown"},
	{EM_CFG_OPEN_DOOR_GROUP_METHOD_CARD,		"Card"},
	{EM_CFG_OPEN_DOOR_GROUP_METHOD_PWD,			"PWD"},
	{EM_CFG_OPEN_DOOR_GROUP_METHOD_FINGERPRINT,	"FingerPrint"},
	//{EM_CFG_OPEN_DOOR_GROUP_METHOD_ANY,			"Any"},
	{EM_CFG_OPEN_DOOR_GROUP_METHOD_FACE,		"Face"},
};

typedef struct __COL_DES
{
	std::string strColTitle; // title of column
	int			nColWidth;   // width of column
}ColDes;

CString ConvertString(CString strText);
std::string ConvertStr(char* szKeyName);
bool GetModulePath(const char* szDllName, char* szModuleDir);
bool GetModulePath(const char* szDllName, std::string& strModuleDir);
std::string GuidToString(const GUID &guid);
std::string Int2Str(const int v);
//std::string VehicleSizeToString(const int v);
std::string NetTime2Str(const NET_TIME& netTime);
std::string NetTimeEx2Str(const NET_TIME_EX& netTime);
std::string NetTimeEx2StrEx(const NET_TIME_EX& netTime);
std::string MakeLP(char* szLP);
void  g_SetWndStaticText(CWnd * pWnd);
std::string GetDataFolder();
BOOL IsTypeHasLP(DWORD dwEventType);
ColDes ColDesObj(char* szColTitle, const int nColWidth);
std::string EventType2Str(int nEventType);
std::string AccessDoorOpenMethod2Str(int nMethod);
std::string AccessAlarmType2Str(int nType);

#endif