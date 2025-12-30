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
// 	{CFG_DOOR_OPEN_METHOD_UNKNOWN, "Unknown"},
// 	{CFG_DOOR_OPEN_METHOD_PWD_ONLY, "PwdOnly"},

	{CFG_DOOR_OPEN_METHOD_UNKNOWN, "UNKNOWN"},
	{CFG_DOOR_OPEN_METHOD_PWD_ONLY, "PWD_ONLY"},
	{CFG_DOOR_OPEN_METHOD_CARD, "CARD"},
	{CFG_DOOR_OPEN_METHOD_PWD_OR_CARD, "PWD_OR_CARD"},
	{CFG_DOOR_OPEN_METHOD_CARD_FIRST, "CARD_FIRST"},
	{CFG_DOOR_OPEN_METHOD_PWD_FIRST, "PWD_FIRST"},
	{CFG_DOOR_OPEN_METHOD_SECTION, "SECTION"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINTONLY, "FINGERPRINTONLY"},
	{CFG_DOOR_OPEN_METHOD_PWD_OR_CARD_OR_FINGERPRINT, "PWD_OR_CARD_OR_FINGERPRINT"},
	{CFG_DOOR_OPEN_METHOD_PWD_AND_CARD_AND_FINGERPINT, "PWD_AND_CARD_AND_FINGERPINT"},
	{CFG_DOOR_OPEN_METHOD_PWD_AND_FINGERPRINT, "PWD_AND_FINGERPRINT"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_FINGERPRINT, "CARD_AND_FINGERPRINT"},
	{CFG_DOOR_OPEN_METHOD_MULTI_PERSON, "MULTI_PERSON"},
	{CFG_DOOR_OPEN_METHOD_FACEIDCARD, "FACEIDCARD"},
	{CFG_DOOR_OPEN_METHOD_FACEIDCARD_AND_IDCARD, "FACEIDCARD_AND_IDCARD"},
	{CFG_DOOR_OPEN_METHOD_FACEIDCARD_OR_CARD_OR_FINGER, "FACEIDCARD_OR_CARD_OR_FINGER"},
	{CFG_DOOR_OPEN_METHOD_FACEIPCARDANDIDCARD_OR_CARD_OR_FINGER, "FACEIPCARDANDIDCARD_OR_CARD_OR_FINGER"},
	{CFG_DOOR_OPEN_METHOD_USERID_AND_PWD, "USERID_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_FACE_ONLY, "FACE_ONLY"},
	{CFG_DOOR_OPEN_METHOD_FACE_AND_PWD, "FACE_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINT_AND_PWD, "FINGERPRINT_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINT_AND_FACE, "FINGERPRINT_AND_FACE"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_FACE, "CARD_AND_FACE"},
	{CFG_DOOR_OPEN_METHOD_FACE_OR_PWD, "FACE_OR_PWD"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINT_OR_PWD, "FINGERPRINT_OR_PWD"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINT_OR_FACE, "FINGERPRINT_OR_FACE"},
	{CFG_DOOR_OPEN_METHOD_CARD_OR_FACE, "CARD_OR_FACE"},
	{CFG_DOOR_OPEN_METHOD_CARD_OR_FINGERPRINT, "CARD_OR_FINGERPRINT"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINT_AND_FACE_AND_PWD, "FINGERPRINT_AND_FACE_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_FACE_AND_PWD, "CARD_AND_FACE_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_FINGERPRINT_AND_PWD, "CARD_AND_FINGERPRINT_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_PWD_AND_FACE, "CARD_AND_PWD_AND_FACE"},
	{CFG_DOOR_OPEN_METHOD_FINGERPRINT_OR_FACE_OR_PWD, "FINGERPRINT_OR_FACE_OR_PWD"},
	{CFG_DOOR_OPEN_METHOD_CARD_OR_FACE_OR_PWD, "CARD_OR_FACE_OR_PWD"},
	{CFG_DOOR_OPEN_METHOD_CARD_OR_FINGERPRINT_OR_FACE, "CARD_OR_FINGERPRINT_OR_FACE"},
	{CFG_DOOR_OPEN_METHOD_CARD_AND_FINGERPRINT_AND_FACE_AND_PWD, "CARD_AND_FINGERPRINT_AND_FACE_AND_PWD"},
	{CFG_DOOR_OPEN_METHOD_CARD_OR_FINGERPRINT_OR_FACE_OR_PWD, "CARD_OR_FINGERPRINT_OR_FACE_OR_PWD"},
	//{CFG_DOOR_OPEN_METHOD_FACEIPCARDANDIDCARD_OR_CARD_OR_FACE, "FACEIPCARDANDIDCARD_OR_CARD_OR_FACE"},
	//{CFG_DOOR_OPEN_METHOD_FACEIDCARD_OR_CARD_OR_FACE, "FACEIDCARD_OR_CARD_OR_FACE"},
	//{CFG_DOOR_OPEN_METHOD_CARDANDPWD_OR_FINGERPRINTANDPWD, "CARDANDPWD_OR_FINGERPRINTANDPWD"},
	//{CFG_DOOR_OPEN_METHOD_PHOTO_OR_FACE, "PHOTO_OR_FACE"},
	//{CFG_DOOR_OPEN_METHOD_FINGERPRINT, "FINGERPRINT"},
	//{CFG_DOOR_OPEN_METHOD_PHOTO_AND_FINGERPRINT, "PHOTO_AND_FINGERPRINT"},

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
std::string FailCodeType2Str(int nType);
std::string UserType2Str(int nUserType);
std::string CardType2Str(int nCardType);
#endif