#include "stdafx.h"
#include "Utility.h"
#include <atlconv.h>
#include <assert.h>

static char pszIniPath[512] = {0};

const CString strEventList[] = {
	"Entry",
	"Exit",
	"NotClose",
	"BreakIn",
	"RepeakIn",
	"Duress",
	"ChassisIntruded",
	"AlarmEx2",
	"Malicious"
};
const int nEventTypeList[]={
	NET_ACCESS_CTL_EVENT_ENTRY,
	NET_ACCESS_CTL_EVENT_EXIT,
	DH_ALARM_ACCESS_CTL_NOT_CLOSE,
	DH_ALARM_ACCESS_CTL_BREAK_IN,
	DH_ALARM_ACCESS_CTL_REPEAT_ENTER,
	DH_ALARM_ACCESS_CTL_DURESS,
	DH_ALARM_CHASSISINTRUDED,
	DH_ALARM_ALARM_EX2,
	DH_ALARM_ACCESS_CTL_MALICIOUS
};

const CString strUserList[] = {
	"Normal",
	"Blocklist",
	"Guest",
	"Patrol",
	"VIP",
	"Handicap",
	"CUSTOM1",
	"CUSTOM2"
};
const int nUserTypeList[]={
	NET_ENUM_USER_TYPE_NORMAL,
	NET_ENUM_USER_TYPE_BLACKLIST,
	NET_ENUM_USER_TYPE_GUEST,
	NET_ENUM_USER_TYPE_PATROL,
	NET_ENUM_USER_TYPE_VIP,
	NET_ENUM_USER_TYPE_HANDICAP,
	NET_ENUM_USER_TYPE_CUSTOM1,
	NET_ENUM_USER_TYPE_CUSTOM2
};

const CString strCardList[] = {
	"Normal",
	"VIP",
	"Guest",
	"Patrol",
	"Blocklist",
	"Corce",
	"Polling",
	"MotherCard"
};
const int nCardTypeList[]={
	NET_ACCESSCTLCARD_TYPE_GENERAL,                     // 一般卡
	NET_ACCESSCTLCARD_TYPE_VIP,                         // VIP卡
	NET_ACCESSCTLCARD_TYPE_GUEST,                       // 来宾卡
	NET_ACCESSCTLCARD_TYPE_PATROL,                      // 巡逻卡
	NET_ACCESSCTLCARD_TYPE_BLACKLIST,                   // 禁止名单卡
	NET_ACCESSCTLCARD_TYPE_CORCE,                       // 胁迫卡
	NET_ACCESSCTLCARD_TYPE_POLLING,                     // 巡检卡
	NET_ACCESSCTLCARD_TYPE_MOTHERCARD
};

const CString strDoorOpenMethodList[] = {
	"UNKNOWN",
	"PWD_ONLY",                    
	"CARD",                       
	"CARD_FIRST",                  
	"PWD_FIRST",                   
	"REMOTE",                      
	"BUTTON",                      
	"FINGERPRINT",                
	"PWD_CARD_FINGERPRINT",        
	"PWD_FINGERPRINT",        
	"CARD_FINGERPRINT",       
	"PERSONS",               
	"KEY",                
	"COERCE_PWD",         
	"QRCODE",              
	"TARG_RECOGNITION",    
	"FACEIDCARD",          
	"FACEIDCARD_AND_IDCARD",
	"BLUETOOTH",			
	"CUSTOM_PASSWORD",    
	"USERID_AND_PWD",  
	"FACE_AND_PWD",     
	"FINGERPRINT_AND_PWD", 
	"FINGERPRINT_AND_FACE", 
	"CARD_AND_FACE",
	"FACE_OR_PWD",     
	"FINGERPRINT_OR_PWD", 
	"FINGERPRINT_OR_FACE", 
	"CARD_OR_FACE",     
	"CARD_OR_FINGERPRINT",  
	"FINGERPRINT_AND_FACE_AND_PWD",   
	"CARD_AND_FACE_AND_PWD",           
	"CARD_AND_FINGERPRINT_AND_PWD",          
	"CARD_AND_PWD_AND_FACE",          
	"FINGERPRINT_OR_FACE_OR_PWD",         
	"CARD_OR_FACE_OR_PWD",              
	"CARD_OR_FINGERPRINT_OR_FACE",        
	"CARD_AND_FINGERPRINT_AND_FACE_AND_PWD",
	"CARD_OR_FINGERPRINT_OR_FACE_OR_PWD", 
	"FACEIPCARDANDIDCARD_OR_CARD_OR_FACE",
	"FACEIDCARD_OR_CARD_OR_FACE",  
	"DTMF",		
	"REMOTE_QRCODE",
	"REMOTE_FACE",	
	"CITIZEN_FINGERPRINT"
};

const int nAccessDoorOpenMethodList[]={
	NET_ACCESS_DOOROPEN_METHOD_UNKNOWN,
	NET_ACCESS_DOOROPEN_METHOD_PWD_ONLY,      
	NET_ACCESS_DOOROPEN_METHOD_CARD,      
	NET_ACCESS_DOOROPEN_METHOD_CARD_FIRST,         
	NET_ACCESS_DOOROPEN_METHOD_PWD_FIRST,            
	NET_ACCESS_DOOROPEN_METHOD_REMOTE,              
	NET_ACCESS_DOOROPEN_METHOD_BUTTON,               
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT,          
	NET_ACCESS_DOOROPEN_METHOD_PWD_CARD_FINGERPRINT,  
	NET_ACCESS_DOOROPEN_METHOD_PWD_FINGERPRINT,   
	NET_ACCESS_DOOROPEN_METHOD_CARD_FINGERPRINT,  
	NET_ACCESS_DOOROPEN_METHOD_PERSONS,           
	NET_ACCESS_DOOROPEN_METHOD_KEY,                
	NET_ACCESS_DOOROPEN_METHOD_COERCE_PWD,          
	NET_ACCESS_DOOROPEN_METHOD_QRCODE,             
	NET_ACCESS_DOOROPEN_METHOD_FACE_RECOGNITION,   
	NET_ACCESS_DOOROPEN_METHOD_FACEIDCARD,          
	NET_ACCESS_DOOROPEN_METHOD_FACEIDCARD_AND_IDCARD,
	NET_ACCESS_DOOROPEN_METHOD_BLUETOOTH,			
	NET_ACCESS_DOOROPEN_METHOD_CUSTOM_PASSWORD,    
	NET_ACCESS_DOOROPEN_METHOD_USERID_AND_PWD,     
	NET_ACCESS_DOOROPEN_METHOD_FACE_AND_PWD ,      
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT_AND_PWD,   
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT_AND_FACE,  
	NET_ACCESS_DOOROPEN_METHOD_CARD_AND_FACE,        
	NET_ACCESS_DOOROPEN_METHOD_FACE_OR_PWD,           
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT_OR_PWD,   
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT_OR_FACE,   
	NET_ACCESS_DOOROPEN_METHOD_CARD_OR_FACE,         
	NET_ACCESS_DOOROPEN_METHOD_CARD_OR_FINGERPRINT,   
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT_AND_FACE_AND_PWD,   
	NET_ACCESS_DOOROPEN_METHOD_CARD_AND_FACE_AND_PWD,            
	NET_ACCESS_DOOROPEN_METHOD_CARD_AND_FINGERPRINT_AND_PWD,       
	NET_ACCESS_DOOROPEN_METHOD_CARD_AND_PWD_AND_FACE,              
	NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT_OR_FACE_OR_PWD,          
	NET_ACCESS_DOOROPEN_METHOD_CARD_OR_FACE_OR_PWD,                  
	NET_ACCESS_DOOROPEN_METHOD_CARD_OR_FINGERPRINT_OR_FACE,            
	NET_ACCESS_DOOROPEN_METHOD_CARD_AND_FINGERPRINT_AND_FACE_AND_PWD,  
	NET_ACCESS_DOOROPEN_METHOD_CARD_OR_FINGERPRINT_OR_FACE_OR_PWD,   
	NET_ACCESS_DOOROPEN_METHOD_FACEIPCARDANDIDCARD_OR_CARD_OR_FACE,  
	NET_ACCESS_DOOROPEN_METHOD_FACEIDCARD_OR_CARD_OR_FACE,       
	NET_ACCESS_DOOROPEN_METHOD_DTMF,				
	NET_ACCESS_DOOROPEN_METHOD_REMOTE_QRCODE ,		
	NET_ACCESS_DOOROPEN_METHOD_REMOTE_FACE ,		
	NET_ACCESS_DOOROPEN_METHOD_CITIZEN_FINGERPRINT	
};

const CString strAccessAlarmType[] = {
	"UNKNOWN",
	"DOOR_NOTCLOSE",
	"BREAK_IN",
	"REPEAT_ENTER",
	"DURESS",
	"ALARMLOCAL",
	"CHASSIS_INTRUDED",
	"MALICIOUS",
	"BLOCKLIST"
};

const int nAccessAlarmTypeList[]={
	EM_RECORD_ACCESS_ALARM_TYPE_UNKNOWN,                       
	EM_RECORD_ACCESS_ALARM_TYPE_DOOR_NOTCLOSE,               
	EM_RECORD_ACCESS_ALARM_TYPE_BREAK_IN,                      
	EM_RECORD_ACCESS_ALARM_TYPE_REPEAT_ENTER,                     
	EM_RECORD_ACCESS_ALARM_TYPE_DURESS,                            
	EM_RECORD_ACCESS_ALARM_TYPE_ALARMLOCAL,                   
	EM_RECORD_ACCESS_ALARM_TYPE_CHASSIS_INTRUDED,
	EM_RECORD_ACCESS_ALARM_TYPE_MALICIOUS,
	EM_RECORD_ACCESS_ALARM_TYPE_BLACKLIST
};

const CString strFailCodeType[] = {
	"NOERROR",                                                // 没有错误
	"UNKNOWN",                                                // 未知错误
	"INVALID_PARAM",                                          // 参数错误
	"INVALID_PASSWORD",                                       // 无效密码
	"INVALID_FP",                                             // 无效指纹数据
	"INVALID_FACE",                                           // 无效人脸数据
	"INVALID_CARD",                                           // 无效卡数据
	"INVALID_USER",                                           // 无效人数据
	"FAILED_GET_SUBSERVICE",                                  // 能力集子服务获取失败
	"FAILED_GET_METHOD",                                      // 获取组件的方法集失败
	"FAILED_GET_SUBCAPS",                                     // 获取资源实体能力集失败
	"ERROR_INSERT_LIMIT",                                     // 已达插入上限
	"ERROR_MAX_INSERT_RATE",                                  // 已达最大插入速度
	"FAILED_ERASE_FP",										// 清除指纹数据失败
	"FAILED_ERASE_FACE",										// 清除人脸数据失败
	"FAILED_ERASE_CARD",										// 清除卡数据失败
	"NO_RECORD",												// 没有记录
	"NOMORE_RECORD",											// 查找到最后，没有更多记录
	"RECORD_ALREADY_EXISTS",									// 下发卡或指纹时，数据重复
	"MAX_FP_PERUSER",											// 超过个人最大指纹记录数
	"MAX_CARD_PERUSER",										// 超过个人最大卡片记录数
	"EXCEED_MAX_PHOTOSIZE",									// 超出最大照片大小
	"INVALID_USERID",											// 用户ID无效（未找到客户）
	"EXTRACTFEATURE_FAIL",									// 提取人脸特征失败
	"PHOTO_EXIST",											// 人脸照片已存在
	"PHOTO_OVERFLOW",											// 超出最大人脸照片数
	"INVALID_PHOTO_FORMAT",									// 照片格式无效
	"EXCEED_ADMINISTRATOR_LIMIT"
};

const int nFailCodeTypeList[]={
	NET_EM_FAILCODE_NOERROR,                                                // 没有错误
	NET_EM_FAILCODE_UNKNOWN,                                                // 未知错误
	NET_EM_FAILCODE_INVALID_PARAM,                                          // 参数错误
	NET_EM_FAILCODE_INVALID_PASSWORD,                                       // 无效密码
	NET_EM_FAILCODE_INVALID_FP,                                             // 无效指纹数据
	NET_EM_FAILCODE_INVALID_FACE,                                           // 无效人脸数据
	NET_EM_FAILCODE_INVALID_CARD,                                           // 无效卡数据
	NET_EM_FAILCODE_INVALID_USER,                                           // 无效人数据
	NET_EM_FAILCODE_FAILED_GET_SUBSERVICE,                                  // 能力集子服务获取失败
	NET_EM_FAILCODE_FAILED_GET_METHOD,                                      // 获取组件的方法集失败
	NET_EM_FAILCODE_FAILED_GET_SUBCAPS,                                     // 获取资源实体能力集失败
	NET_EM_FAILCODE_ERROR_INSERT_LIMIT,                                     // 已达插入上限
	NET_EM_FAILCODE_ERROR_MAX_INSERT_RATE,                                  // 已达最大插入速度
	NET_EM_FAILCODE_FAILED_ERASE_FP,										// 清除指纹数据失败
	NET_EM_FAILCODE_FAILED_ERASE_FACE,										// 清除人脸数据失败
	NET_EM_FAILCODE_FAILED_ERASE_CARD,										// 清除卡数据失败
	NET_EM_FAILCODE_NO_RECORD,												// 没有记录
	NET_EM_FAILCODE_NOMORE_RECORD,											// 查找到最后，没有更多记录
	NET_EM_FAILCODE_RECORD_ALREADY_EXISTS,									// 下发卡或指纹时，数据重复
	NET_EM_FAILCODE_MAX_FP_PERUSER,											// 超过个人最大指纹记录数
	NET_EM_FAILCODE_MAX_CARD_PERUSER,										// 超过个人最大卡片记录数
	NET_EM_FAILCODE_EXCEED_MAX_PHOTOSIZE,									// 超出最大照片大小
	NET_EM_FAILCODE_INVALID_USERID,											// 用户ID无效（未找到客户）
	NET_EM_FAILCODE_EXTRACTFEATURE_FAIL,									// 提取人脸特征失败
	NET_EM_FAILCODE_PHOTO_EXIST,											// 人脸照片已存在
	NET_EM_FAILCODE_PHOTO_OVERFLOW,											// 超出最大人脸照片数
	NET_EM_FAILCODE_INVALID_PHOTO_FORMAT,									// 照片格式无效
	NET_EM_FAILCODE_EXCEED_ADMINISTRATOR_LIMIT
};

CString ConvertString(CString strText)
{
	char *val = NEW char[200];
	CString strIniPath,strRet;

	if( strlen(pszIniPath) == 0 )
	{
		char szDirBuf[512] = {0};
		GetCurrentDirectory(512, szDirBuf);
		_snprintf(pszIniPath, sizeof(pszIniPath), "%s\\langchn.ini", szDirBuf);
	}

	memset(val,0,200);
	GetPrivateProfileString("String",strText,"",
		val,200,pszIniPath);
	strRet = val;
	if(strRet.GetLength()==0)
	{
		//If there is no corresponding string in ini file then set it to be default value(English).
		strRet=strText;
	}
	delete[] val;
	return strRet;
}

std::string ConvertStr(char* szKeyName)
{
	using namespace std;
	static bool bFirstTime = true;
	static map<std::string, std::string> g_text;
	if( strlen(pszIniPath) == 0 )
	{
		char szDirBuf[512] = {0};
		GetCurrentDirectory(512, szDirBuf);
		_snprintf(pszIniPath, sizeof(pszIniPath), "%s\\langchn.ini", szDirBuf);
	}

	if ( bFirstTime )
	{
		bFirstTime = false;
		const int MAX_ALL_KEY_SIZE = 1024*50;
		const int MAX_SINGLE_KEY_SIZE = 1024;
		const int MAX_SINGLE_VAL_SIZE = 1024;
		char szKeys[MAX_ALL_KEY_SIZE] = {0};
		DWORD dwRet = GetPrivateProfileString("String",
			NULL,
			NULL,
			szKeys,
			MAX_ALL_KEY_SIZE,
			pszIniPath);
		if (dwRet != 0)
		{
			char* szKey = szKeys;
			char szVal[MAX_SINGLE_VAL_SIZE]; 
			DWORD dwProcessCount = 0;
			while(true)
			{
				memset(szVal, 0, MAX_SINGLE_VAL_SIZE);
				GetPrivateProfileString("String",
					szKey,
					szKey,
					szVal,
					MAX_SINGLE_VAL_SIZE,
					pszIniPath);
				g_text.insert(pair<std::string, std::string>(szKey, szVal));

				szKey += (strlen(szKey) + 1);
				size_t nCount = szKey - szKeys;
				if (nCount >= dwRet)
				{
					break;
				}
			}
		}
	}

	string strRet = g_text[szKeyName];
	if (strRet.size() != 0)
	{
		return strRet;
	}
	else
	{
		return string(szKeyName);
	}
}

bool GetModulePath(const char* szDllName, char* szModuleDir)
{
	HMODULE hModule = NULL;
	if (szDllName != NULL)
	{
		hModule = ::GetModuleHandle(szDllName);
		if (hModule == NULL)
		{
			return false;
		}
	}
	char szFullDllPath[_MAX_PATH] = {0};
	::GetModuleFileName(hModule, szFullDllPath, _MAX_PATH);
	char* lastBackslash = strrchr(szFullDllPath, '\\');
	size_t lastBackslashPos = lastBackslash - szFullDllPath + 1;
	strncpy(szModuleDir, szFullDllPath, lastBackslashPos);
	return true;
}


bool GetModulePath(const char* szDllName, std::string& strModuleDir)
{
	char buf[MAX_PATH] = {0};
	bool bRet = GetModulePath(szDllName, buf);
	if (bRet)
	{
		strModuleDir = std::string(buf);
		return true;
	}
	return false;
}

/************************************************************************/
/* GUID to std::string                                                  */
/************************************************************************/
std::string GuidToString(const GUID &guid)
{
	char buf[64] = {0};
	_snprintf(
		buf,
		sizeof(buf),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return std::string(buf);
}

std::string Int2Str(const int v)
{
	char szVal[20] = {0};
	_itoa(v, szVal, 10);
	return std::string(szVal);
}
/*
std::string VehicleSizeToString(const int v)
{
	std::string strType = "Unknown";
	if (v & 0x1)
	{
		strType = ConvertString("Light-duty"); 
	}
	if (v & 0x2)
	{
		strType = ConvertStr("Medium"); 
	}
	if (v & 0x4)
	{
		strType = ConvertStr("Oversize"); 
	}
	if (v & 0x8)
	{
		strType = ConvertStr("Minisize"); 
	}
	if (v & 0x16)
	{
		strType = ConvertStr("Largesize"); 
	}
	return strType;	
}*/

/************************************************************************/
/* NET_TIME to std::string                                              */
/************************************************************************/
std::string NetTime2Str(const NET_TIME& netTime)
{
	char buf[128] = {0};
	_snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", 
		netTime.dwYear,
		netTime.dwMonth,
		netTime.dwDay,
		netTime.dwHour,
		netTime.dwMinute,
		netTime.dwSecond
		);
	return std::string(buf);
}

/************************************************************************/
/* NET_TIME to std::string                                              */
/************************************************************************/
std::string NetTimeEx2Str(const NET_TIME_EX& netTime)
{
	char buf[128] = {0};
	_snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
		netTime.dwYear,
		netTime.dwMonth,
		netTime.dwDay,
		netTime.dwHour,
		netTime.dwMinute,
		netTime.dwSecond,
		netTime.dwMillisecond
		);
	return std::string(buf);
}

/************************************************************************/
/* NET_TIME to std::string                                              */
/************************************************************************/
std::string NetTimeEx2StrEx(const NET_TIME_EX& netTime)
{
	char buf[128] = {0};
	_snprintf(buf, sizeof(buf), "%04d%02d%02d%02d%02d%02d%03d",
		netTime.dwYear,
		netTime.dwMonth,
		netTime.dwDay,
		netTime.dwHour,
		netTime.dwMinute,
		netTime.dwSecond,
		netTime.dwMillisecond);
	return std::string(buf);
}

std::string MakeLP(char* szLP)
{
	if (strcmp(szLP, "") == 0)
	{
		std::string strRet = ConvertStr("");
		return strRet;
	}
	else
	{
		return std::string(szLP);
	}
}

ColDes ColDesObj(char* szColTitle, const int nColWidth)
{
	ColDes colDes;
	colDes.strColTitle = ConvertStr(szColTitle);
	colDes.nColWidth = nColWidth;
	return colDes;
}

//Set static text for dialog box(English->current language)
void g_SetWndStaticText(CWnd * pWnd)
{
	CString strCaption,strText;

	//Set main window title 
	pWnd->GetWindowText(strCaption);
	if(strCaption.GetLength()>0)
	{
		strText=ConvertString(strCaption);
		pWnd->SetWindowText(strText);
	}

	//Set small window title
	CWnd * pChild=pWnd->GetWindow(GW_CHILD);
	CString strClassName;
	while(pChild)
	{
		strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
		if(strClassName == "CEdit")
		{
			//The next small window 
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}	

		//Set current language text for small window. 
		pChild->GetWindowText(strCaption);
		strText=ConvertString(strCaption);
		pChild->SetWindowText(strText);

		//Next small window 
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}

BOOL IsTypeHasLP(DWORD dwEventType)
{
	for (int i = 1; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		if (dwEventType == nEventTypeList[i])
		{
			return TRUE;
		}
	}
	return FALSE;
}

std::string GetDataFolder()
{
	std::string strCfgFilePath;
	std::string strCfgDefaultFolder;
	GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append(CFG_CFG_FILE_NAME);
	strCfgDefaultFolder.append(PC_DATA_STORE_FOLDER);
	char szFolder[1024] = {0};
	::GetPrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), szFolder, 1024, strCfgFilePath.c_str());
	return std::string(szFolder);
}

std::string EventType2Str(int nEventType)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 0; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		if (nEventType == nEventTypeList[i])
		{
			strType = ConvertString(strEventList[i]);
		}
	}
	return strType;
}

std::string AccessDoorOpenMethod2Str(int nMethod)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 1; i < sizeof(nAccessDoorOpenMethodList)/sizeof(int); i++)
	{	
		if (nMethod == nAccessDoorOpenMethodList[i])
		{
			strType = ConvertString(strDoorOpenMethodList[i]);
		}
	}
	return strType;
}

std::string AccessAlarmType2Str(int nType)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 1; i < sizeof(nAccessAlarmTypeList)/sizeof(int); i++)
	{	
		if (nType == nAccessAlarmTypeList[i])
		{
			strType = ConvertString(strAccessAlarmType[i]);
		}
	}
	return strType;
}

std::string FailCodeType2Str(int nType)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 1; i < sizeof(nFailCodeTypeList)/sizeof(int); i++)
	{	
		if (nType == nFailCodeTypeList[i])
		{
			strType = ConvertString(strFailCodeType[i]);
		}
	}
	return strType;
}

std::string UserType2Str(int nUserType)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 0; i < sizeof(nUserTypeList)/sizeof(int); i++)
	{	
		if (nUserType == nUserTypeList[i])
		{
			strType = ConvertString(strUserList[i]);
		}
	}
	return strType;
}

std::string CardType2Str(int nCardType)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 0; i < sizeof(nCardTypeList)/sizeof(int); i++)
	{	
		if (nCardType == nCardTypeList[i])
		{
			strType = ConvertString(strCardList[i]);
		}
	}
	return strType;
}
