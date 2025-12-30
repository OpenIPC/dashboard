#include "stdafx.h"
#include "Utility.h"
#include <atlconv.h>
#include <assert.h>

static char pszIniPath[512] = {0};

const CString strEventList[] = {
	"Show All",
	"Junction",
	"ManualSnap",
	"TrafficVehiclePosition"
};
const int nEventTypeList[]={
	EVENT_IVS_ALL, 
	EVENT_IVS_TRAFFICJUNCTION,
	EVENT_IVS_TRAFFIC_MANUALSNAP,
	DH_ALARM_TRAFFIC_VEHICLE_POSITION
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
	for (int i = 1; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		if (nEventType == nEventTypeList[i])
		{
			strType = ConvertString(strEventList[i]);
		}
	}
	return strType;
}

CString ShowErr(EM_DEVICE_ERR emDevErr)
{
	CString str;
	switch(emDevErr)
	{
	case EM_UNKNOWN_ERR:
		str = "unknown";
		break;
	case EM_LOGIN_FAIL:
		str = "Login failed!";
		break;
	case EM_WITHOUT_LOGIN:
		str = "Not login yet!";
		break;
	case EM_OPENSTROBE_FAIL:
		str = "Open Strobe fail";
		break;
	case EM_SNAP_FAIL:
		str = "Manual Snap failed!";
		break;
	case EM_SCRIBEPIC_FAIL:
		str = "Subscribe picture failed!";
		break;
	case EM_PLAY_FAIL:
		str = "Fail to play!";
		break;
	case EM_WITHOUT_SCRIBEPIC:
		str = "Please start subscribe intelligent events before manual snap";
		break;
	case EM_LISTEN_FAIL:
		str = "Subscribe failed";
		break;
	case EM_GET_STROBE_FAIL:
		str = "setconfig error";
		break;
	case EM_SET_STROBE_FAIL:
		str = "setconfig error";
		break;
	case EM_SET_LATTICESCREEN_FAIL:
		str = "setconfig error";
		break;
	case EM_IMPORTLIST_FAIL:
		str = "Transmit Error";
		break;
	case EM_EXPORTLIST_FAIL:
		str = "Transmit Error";
		break;
	case EM_SET_ENCODE_FAIL:
		str = "Set encode fail!";
		break;
	case EM_SET_TRANSFERMODE_FAIL:
		str = "Set transfer mode fail!";
		break;
	case EM_START_RECORD_FAIL:
		str = "Start local record failed!";
		break;
	case EM_START_TALK_FAIL:
		str = "Open talk failed!";
		break;
	case EM_STOP_TALK_FAIL:
		str = "Stop talk failed!";
		break;
	default:
		str = "unknown";
		break;
	}
	return str;
 }