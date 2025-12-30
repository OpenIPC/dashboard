#if !defined(_DEVICE_)
#define _DEVICE_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum EM_DEVICE_ERR
{
	EM_UNKNOWN_ERR,
	EM_LOGIN_FAIL,
	EM_WITHOUT_LOGIN,
	EM_OPENSTROBE_FAIL,
	EM_SNAP_FAIL,
	EM_SCRIBEPIC_FAIL,
	EM_PLAY_FAIL,
	EM_WITHOUT_SCRIBEPIC,
	EM_LISTEN_FAIL,
	EM_GET_STROBE_FAIL,
	EM_SET_STROBE_FAIL,
	EM_SET_LATTICESCREEN_FAIL,
	EM_IMPORTLIST_FAIL,
	EM_EXPORTLIST_FAIL,
	EM_SET_ENCODE_FAIL,
	EM_SET_TRANSFERMODE_FAIL,
	EM_START_RECORD_FAIL,
	EM_START_TALK_FAIL,
	EM_STOP_TALK_FAIL
};
class DeviceImpl;
class Device
{
public:
	static Device &GetInstance();
	void Destroy();

	void SetReconnect(fDisConnect cbDisConnect, fHaveReConnect cbAutoConnect, fSubDisConnect cbSubDisConnect, LDWORD dwUser);

	BOOL Login(const char *pchDVRIP, WORD wDVRPort, const char *pchUserName, const char *pchPassword);
	BOOL Logout();

	BOOL StartRealPlay(int nChannelID, HWND hWnd);
	BOOL StopRealPlay();

	BOOL StrobeControl(CtrlType emType, int nSelChan);
	BOOL ManualSnap(CtrlType emType, int nSelChan);

	BOOL StartRealLoadPicture(int nChannelID, DWORD dwAlarmType, BOOL bNeedPicFile, fAnalyzerDataCallBack cbAnalyzerData, LDWORD dwUser/*, void* Reserved*/);
	BOOL StopRealLoadPicture();

	BOOL StartListenEvent(fMessCallBack cbMessage,LDWORD dwUser);
	BOOL StopListenEvent();

	BOOL SetStrobe(NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,void* szInBuffer, DWORD dwOutBufferSize);
	BOOL GetStrobe(NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,void* szOutBuffer, DWORD dwInBufferSize);

	BOOL SetLatticeScreen(CtrlType emType, void* pInBuf);

	BOOL StartImportList(char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, LDWORD dwUserData);  
	BOOL StopImportList();

	BOOL StartExportList(char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, LDWORD dwUserData);  
	BOOL StopExportList();

	BOOL StartTalk(pfAudioDataCallBack pfcb, LDWORD dwUser);
	BOOL StopTalk();
	BOOL AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag);

	BOOL GetLoginState();
	int  GetChanNum();

	BOOL SetParkControlInfo(void* pInBuf, void* pOutBuf);

	EM_DEVICE_ERR GetDeviceErr();
private:
	Device();
	~Device();
	Device(const Device& device);
	Device& operator=(const Device& device);
	static Device*	mInstance;
private:
	
	DeviceImpl*			m_pDevImpl;
};
#endif