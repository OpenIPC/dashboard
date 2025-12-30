#ifndef _UTILITY_H_
#define _UTILITY_H_

#define MAX_EVENT_NUM 100

CString ConvertString(CString strText);

std::string ConvertStr(char* szKeyName);
/************************************************************************/
/* Get the absolute path of the module                                               */
/************************************************************************/
bool GetModulePath(const char* szDllName, char* szModuleDir);

/************************************************************************/
/* Get the absolute path of the module                                               */
/************************************************************************/
bool GetModulePath(const char* szDllName, std::string& strModuleDir);


/************************************************************************/
/* GUID to std::string                                                  */
/************************************************************************/
std::string GuidToString(const GUID &guid);

std::string Int2Str(const int v);

std::string VehicleSizeToString(const int v);

/************************************************************************/
/* NET_TIME to std::string                                              */
/************************************************************************/
std::string NetTimeEx2Str(const NET_TIME_EX& netTime);

/************************************************************************/
/* NET_TIME to std::string                                              */
/************************************************************************/
std::string NetTimeEx2StrEx(const NET_TIME_EX& netTime);

std::string MakeLP(char* szLP);


/************************************************************************/
/* Set static text for dialog box(English->current language)                                             */
/************************************************************************/
void  g_SetWndStaticText(CWnd * pWnd);


std::string GetDataFolder();

/************************************************************************/
/* Debug output print                                             */
/************************************************************************/

std::string EventType2Str(int nEventType);

#endif