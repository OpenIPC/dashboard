
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
m_APIGetPicBMPEx(NULL),
m_APIRenderPrivateData(NULL),
m_APISetStreamOpenMode(NULL),
m_APICatchPicEx(NULL),
m_APIResetBuffer(NULL),
m_APIGetBufferValue(NULL),
m_APIGetSourceBufferRemain(NULL),
m_APIPause(NULL),
m_APISetPlayDirection(NULL),
m_APISetPlaySpeed(NULL),
m_APIOneByOne(NULL)
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
	HMODULE hLib = ::LoadLibraryEx(_T("play.dll"), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
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
		m_APIRenderPrivateData = (PLAY_API_RenderPrivateData)GetProcAddress(hLib,"PLAY_RenderPrivateData");
		m_APIGetPicBMPEx = (PLAY_API_GetPicBMPEx)GetProcAddress(hLib,"PLAY_GetPicBMPEx");
		m_APISetStreamOpenMode = (PLAY_API_SetStreamOpenMode)GetProcAddress(hLib,"PLAY_SetStreamOpenMode");
		m_APICatchPicEx = (PLAY_API_CatchPicEx)GetProcAddress(hLib,"PLAY_CatchPicEx");
		m_APIResetBuffer = (PLAY_API_ResetBuffer)GetProcAddress(hLib,"PLAY_ResetBuffer");
		m_APIGetBufferValue = (PLAY_API_GetBufferValue)GetProcAddress(hLib,"PLAY_GetBufferValue");
		m_APIGetSourceBufferRemain = (PLAY_API_GetSourceBufferRemain)GetProcAddress(hLib,"PLAY_GetSourceBufferRemain");
		m_APIPause = (PLAY_API_Pause)GetProcAddress(hLib,"PLAY_Pause");
		m_APISetPlayDirection = (PLAY_API_SetPlayDirection)GetProcAddress(hLib,"PLAY_SetPlayDirection");
		m_APISetPlaySpeed = (PLAY_API_SetPlaySpeed)GetProcAddress(hLib,"PLAY_SetPlaySpeed");
		m_APIOneByOne = (PLAY_API_OneByOne)GetProcAddress(hLib, "PLAY_OneByOne");
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

BOOL CPlayAPI::PLAY_RenderPrivateData(LONG nPort, BOOL bTrue, LONG nReserve)
{
	if (m_APIRenderPrivateData)
	{
		return m_APIRenderPrivateData(nPort, bTrue, nReserve);
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


BOOL CPlayAPI::PLAY_GetPicBMPEx(LONG nPort, PBYTE pBmpBuf, DWORD dwBufSize, DWORD* pBmpSize, LONG nWidth, LONG nHeight, int nRgbType)
{
	if (m_APIGetPicBMPEx)
	{
		return m_APIGetPicBMPEx(nPort, pBmpBuf, dwBufSize,  pBmpSize,  nWidth, nHeight, nRgbType);
	}
	return FALSE;
}


BOOL CPlayAPI::PLAY_SetStreamOpenMode(LONG nPort,DWORD nMode)
{
	if (m_APISetStreamOpenMode)
	{
		return m_APISetStreamOpenMode(nPort, nMode);
	}
	return FALSE;
}

BOOL CPlayAPI::PLAY_CatchPicEx(LONG nPort,char* sFileName,tPicFormats ePicfomat)
{
	if (m_APICatchPicEx)
	{
		return m_APICatchPicEx(nPort, sFileName, ePicfomat);
	}

	return FALSE;
}
BOOL CPlayAPI::PLAY_ResetBuffer(LONG nPort,DWORD nBufType)
{
	if (m_APIResetBuffer)
	{
		return m_APIResetBuffer(nPort, nBufType);
	}
	return FALSE;
}

DWORD	CPlayAPI::PLAY_GetBufferValue(LONG nPort,DWORD nBufType)
{
	if (m_APIGetBufferValue)
	{
		return m_APIGetBufferValue(nPort, nBufType);
	}
	return 0;
}
DWORD	CPlayAPI::PLAY_GetSourceBufferRemain(LONG nPort)
{
	if (m_APIGetSourceBufferRemain)
	{
		return m_APIGetSourceBufferRemain(nPort);
	}
	return 0;
}
BOOL	CPlayAPI::PLAY_Pause(LONG nPort,DWORD nPause)
{
	if (m_APIPause)
	{
		return m_APIPause(nPort, nPause);
	}
	return FALSE;
}
BOOL	CPlayAPI::PLAY_SetPlayDirection(LONG nPort, DWORD emDirection)
{
	if (m_APISetPlayDirection)
	{
		return m_APISetPlayDirection(nPort, emDirection);
	}
	return FALSE;
}

BOOL CPlayAPI::PLAY_SetPlaySpeed(LONG nPort, float fCoff)
{
	if (m_APISetPlaySpeed)
	{
		return m_APISetPlaySpeed(nPort, fCoff);
	}
	return FALSE;
}

BOOL CPlayAPI::PLAY_OneByOne(LONG nPort)
{
	if (m_APIOneByOne)
	{
		return m_APIOneByOne(nPort);
	}
	return FALSE;
}