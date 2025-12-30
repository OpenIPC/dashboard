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
std::string VehicleSizeToString(const int v);
std::string NetTimeEx2Str(const NET_TIME_EX& netTime);
std::string NetTimeEx2StrEx(const NET_TIME_EX& netTime);
std::string MakeLP(char* szLP);
void  g_SetWndStaticText(CWnd * pWnd);
std::string GetDataFolder();
BOOL IsTypeHasLP(DWORD dwEventType);
ColDes ColDesObj(char* szColTitle, const int nColWidth);
std::string EventType2Str(int nEventType);
CString ShowErr(EM_DEVICE_ERR emDevErr);

#endif