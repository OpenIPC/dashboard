
#include "StdAfx.h"
#include "PlayApi.h"

//////////////////////////////////////////////////////////////////////////
//

CPlayAPI::CPlayAPI():
m_hModule(NULL),
m_APIOpenStream(NULL),
m_APISetDisplayCallBack(NULL),
m_APIRigisterDrawFun(NULL),
m_APIPlay(NULL),
m_APIInputData(NULL),
m_APIQueryInfo(NULL),
m_APIStop(NULL),
m_APICloseStream(NULL),
m_APIGetLastError(NULL)
{
}

CPlayAPI::~CPlayAPI()
{
	if (m_hModule)
	{
		FreeLibrary(m_hModule);
		m_hModule = NULL;

		OutputDebugString(_T("Unload PlaySDK!\n"));
	}
}


//////////////////////////////////////////////////////////////////////////
//

void CPlayAPI::LoadPlayDll()
{
	HMODULE hLib = ::LoadLibraryEx("play.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (hLib)
	{
		m_APIOpenStream = (PLAY_API_OPENSTREAM)GetProcAddress(hLib, "PLAY_OpenStream");;
		m_APISetDisplayCallBack = (PLAY_API_SETIVSCALLBACK)GetProcAddress(hLib, "PLAY_SetIVSCallBack");
		m_APIRigisterDrawFun = (PLAY_API_RIGISTERDRAWFUN)GetProcAddress(hLib, "PLAY_RigisterDrawFun");
		m_APIPlay = (PLAY_API_PLAY)GetProcAddress(hLib, "PLAY_Play");
		m_APIInputData = (PLAY_API_INPUTDATA)GetProcAddress(hLib, "PLAY_InputData");
		m_APIQueryInfo = (PLAY_API_QUERYINFO)GetProcAddress(hLib, "PLAY_QueryInfo");
		m_APIStop = (PLAY_API_STOP)GetProcAddress(hLib, "PLAY_Stop");
		m_APICloseStream = (PLAY_API_CLOSESTREAM)GetProcAddress(hLib, "PLAY_CloseStream");
		m_APIGetLastError = (PLAY_API_GetLastError)GetProcAddress(hLib, "PLAY_GetLastError");

		m_hModule = hLib;
		OutputDebugString(_T("Load PlaySDK Successfully!\n"));
	}
	else
	{
		OutputDebugString(_T("Load PlaySDK Failed!\n"));
	}
}

BOOL CPlayAPI::PLAY_OpenStream(LONG nPort,PBYTE pFileHeadBuf,DWORD nSize,DWORD nBufPoolSize)
{
	if (m_APIOpenStream)
	{
		return m_APIOpenStream(nPort, pFileHeadBuf, nSize, nBufPoolSize);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_SetIVSCallBack(LONG nPort, GetIVSInfoCallbackFunc pFunc, void* pUserData)
{
	if (m_APISetDisplayCallBack)
	{
		return m_APISetDisplayCallBack(nPort, pFunc, pUserData);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_RigisterDrawFun(LONG nPort, DrawFun DrawFuncb, void* pUserData)
{
	if (m_APIRigisterDrawFun)
	{
		return m_APIRigisterDrawFun(nPort, DrawFuncb, pUserData);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_Play(LONG nPort, HWND hWnd)
{
	if (m_APIPlay)
	{
		return m_APIPlay(nPort, hWnd);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_InputData(LONG nPort, PBYTE pBuf, DWORD nSize)
{
	if (m_APIInputData)
	{
		return m_APIInputData(nPort, pBuf, nSize);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_QueryInfo(LONG nPort , int cmdType, char* buf, int buflen, int* returnlen)
{
	if (m_APIQueryInfo)
	{
		return m_APIQueryInfo(nPort, cmdType, buf, buflen, returnlen);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_Stop(LONG nPort)
{
	if (m_APIStop)
	{
		return m_APIStop(nPort);
	}

	return FALSE;
}

BOOL CPlayAPI::PLAY_CloseStream(LONG nPort)
{
	if (m_APICloseStream)
	{
		return m_APICloseStream(nPort);
	}

	return FALSE;
}


DWORD CPlayAPI::PLAY_GetLastError(LONG nPort)
{
	if (m_APIGetLastError)
	{
		return m_APIGetLastError(nPort);
	}
	return -1;
}
