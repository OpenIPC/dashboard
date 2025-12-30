#ifndef _PLAYAPI_HEAD_
#define _PLAYAPI_HEAD_
#include "dhplay.h"

//////////////////////////////////////////////////////////////////////////
// PlaySDK API

typedef void (__stdcall *GetIVSInfoCallbackFunc)(char* buf, long type, long len, long reallen, void* reserved, void* nUser);
typedef void (CALLBACK *DrawFun)(long nPort,HDC hDc,LONG nUser);

typedef BOOL (__stdcall *PLAY_API_OPENSTREAM)(LONG nPort, PBYTE pFileHeadBuf, DWORD nSize, DWORD nBufPoolSize);
typedef BOOL (__stdcall *PLAY_API_SETIVSCALLBACK)(LONG nPort, GetIVSInfoCallbackFunc pFunc, void* pUserData);
typedef BOOL (__stdcall *PLAY_API_RIGISTERDRAWFUN)(LONG nPort, DrawFun DrawFuncb, void* pUserData);
typedef BOOL (__stdcall *PLAY_API_PLAY)(LONG nPort, HWND hWnd);
typedef BOOL (__stdcall *PLAY_API_INPUTDATA)(LONG nPort, PBYTE pBuf, DWORD nSize);
typedef BOOL (__stdcall *PLAY_API_QUERYINFO)(LONG nPort , int cmdType, char* buf, int buflen, int* returnlen);
typedef BOOL (__stdcall *PLAY_API_STOP)(LONG nPort);
typedef BOOL (__stdcall *PLAY_API_CLOSESTREAM)(LONG nPort);
typedef BOOL (__stdcall *PLAY_API_RenderPrivateData)(LONG nPort, BOOL bTrue, LONG nReserve);
typedef BOOL (__stdcall *PLAY_API_GetPicBMPEx)(LONG nPort, PBYTE pBmpBuf, DWORD dwBufSize, DWORD* pBmpSize, LONG nWidth, LONG nHeight, int nRgbType);
typedef BOOL (__stdcall *PLAY_API_SetStreamOpenMode)(LONG nPort,DWORD nMode);
typedef BOOL (__stdcall *PLAY_API_CatchPicEx)(LONG nPort,char* sFileName,tPicFormats ePicfomat);
typedef BOOL (__stdcall *PLAY_API_ResetBuffer)(LONG nPort,DWORD nBufType);
typedef DWORD (__stdcall *PLAY_API_GetBufferValue)(LONG nPort,DWORD nBufType);
typedef DWORD (__stdcall *PLAY_API_GetSourceBufferRemain)(LONG nPort);
typedef BOOL (__stdcall *PLAY_API_Pause)(LONG nPort,DWORD nPause);
typedef BOOL (__stdcall *PLAY_API_SetPlayDirection)(LONG nPort, DWORD emDirection);
typedef BOOL (__stdcall *PLAY_API_SetPlaySpeed)(LONG nPort, float fCoff);
typedef BOOL (__stdcall *PLAY_API_OneByOne)(LONG nPort);


class CPlayAPI
{
public:
	CPlayAPI();
	virtual ~CPlayAPI();

public:
	void LoadPlayDll();

	// PLAY_OpenStream
	BOOL	PLAY_OpenStream(LONG nPort, PBYTE pFileHeadBuf, DWORD nSize, DWORD nBufPoolSize);

	// PLAY_SetIVSCallBack
	BOOL	PLAY_SetIVSCallBack(LONG nPort, GetIVSInfoCallbackFunc pFunc, void* pUserData);

	// PLAY_RigisterDrawFun
	BOOL	PLAY_RigisterDrawFun(LONG nPort, DrawFun DrawFuncb, void* pUserData);

	// PLAY_Play
	BOOL	PLAY_Play(LONG nPort, HWND hWnd);

	// PLAY private data render;display intelligent info
	BOOL	PLAY_RenderPrivateData(LONG nPort, BOOL bTrue, LONG nReserve);

	// PLAY_InputData
	BOOL	PLAY_InputData(LONG nPort, PBYTE pBuf, DWORD nSize);

	// PLAY_QueryInfo
	BOOL	PLAY_QueryInfo(LONG nPort , int cmdType, char* buf, int buflen, int* returnlen);

	// PLAY_Stop
	BOOL	PLAY_Stop(LONG nPort);

	// PLAY_CloseStream
	BOOL	PLAY_CloseStream(LONG nPort);

	BOOL	PLAY_GetPicBMPEx(LONG nPort, PBYTE pBmpBuf, DWORD dwBufSize, DWORD* pBmpSize, LONG nWidth, LONG nHeight, int nRgbType);

	BOOL	PLAY_SetStreamOpenMode(LONG nPort,DWORD nMode);

	BOOL    PLAY_CatchPicEx(LONG nPort,char* sFileName,tPicFormats ePicfomat);
	BOOL	PLAY_ResetBuffer(LONG nPort,DWORD nBufType);

	DWORD	PLAY_GetBufferValue(LONG nPort,DWORD nBufType);
	DWORD	PLAY_GetSourceBufferRemain(LONG nPort);
	BOOL	PLAY_Pause(LONG nPort,DWORD nPause);
	BOOL	PLAY_SetPlayDirection(LONG nPort, DWORD emDirection);
	BOOL	PLAY_SetPlaySpeed(LONG nPort, float fCoff);
	BOOL	PLAY_OneByOne(LONG nPort);

private:
	HMODULE							m_hModule;

	PLAY_API_OPENSTREAM				m_APIOpenStream;
	PLAY_API_SETIVSCALLBACK			m_APISetDisplayCallBack;
	PLAY_API_RIGISTERDRAWFUN		m_APIRigisterDrawFun;
	PLAY_API_PLAY					m_APIPlay;
	PLAY_API_INPUTDATA				m_APIInputData;
	PLAY_API_QUERYINFO				m_APIQueryInfo;
	PLAY_API_STOP					m_APIStop;
	PLAY_API_CLOSESTREAM			m_APICloseStream;
	PLAY_API_RenderPrivateData		m_APIRenderPrivateData;
	PLAY_API_GetPicBMPEx			m_APIGetPicBMPEx;
	PLAY_API_SetStreamOpenMode		m_APISetStreamOpenMode;

	PLAY_API_CatchPicEx				m_APICatchPicEx;
	PLAY_API_ResetBuffer			m_APIResetBuffer;

	PLAY_API_GetBufferValue			m_APIGetBufferValue;
	PLAY_API_GetSourceBufferRemain	m_APIGetSourceBufferRemain;
	PLAY_API_Pause					m_APIPause;
	PLAY_API_SetPlayDirection		m_APISetPlayDirection;
	PLAY_API_SetPlaySpeed			m_APISetPlaySpeed;
	PLAY_API_OneByOne				m_APIOneByOne;
};
#endif