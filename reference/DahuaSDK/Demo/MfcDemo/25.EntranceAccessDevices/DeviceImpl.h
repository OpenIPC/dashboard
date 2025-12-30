#if !defined(_DEVICEIMPL_)
#define _DEVICEIMPL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Device.h"
class DeviceImpl
{
private:
	DeviceImpl(void);
	~DeviceImpl(void);

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
	friend class Device;
	LLONG				m_lLoginID;
	NET_DEVICEINFO_Ex	stunetDevInfo;
	LLONG				m_lPlayID;
	LLONG				m_lRealLoadPicHandle;
	LLONG				m_nTransmitImportHandle;
	LLONG				m_nTransmitExportHandle;
	LLONG               m_hTalkHandle;
	EM_DEVICE_ERR		m_emDevErr;
};
#endif