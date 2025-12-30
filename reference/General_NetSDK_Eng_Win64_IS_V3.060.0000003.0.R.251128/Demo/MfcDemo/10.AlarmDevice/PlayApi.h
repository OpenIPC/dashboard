#ifndef _PLAYAPI_HEAD_
#define _PLAYAPI_HEAD_

//////////////////////////////////////////////////////////////////////////
// PlaySDK API
/*
PLAY_OpenStream()
PLAY_SetIVSCallBack()
PLAY_RigisterDrawFun()
PLAY_Play()
PLAY_InputData()
PLAY_QueryInfo()
PLAY_Stop()
PLAY_CloseStream()
*/

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

	// PLAY_InputData
	BOOL	PLAY_InputData(LONG nPort, PBYTE pBuf, DWORD nSize);

	// PLAY_QueryInfo
	BOOL	PLAY_QueryInfo(LONG nPort , int cmdType, char* buf, int buflen, int* returnlen);

	// PLAY_Stop
	BOOL	PLAY_Stop(LONG nPort);

	// PLAY_CloseStream
	BOOL	PLAY_CloseStream(LONG nPort);

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
};
#endif	// _PLAYAPI_HEAD_