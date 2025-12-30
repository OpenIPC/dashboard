#include "stdafx.h"
#include "Device.h"
#include "DeviceImpl.h"

#ifndef NEW
#define NEW new(std::nothrow)
#endif

#define IMPL_IS_OK	\
	if (m_pDevImpl == NULL)\
	{\
		return FALSE;\
	}

Device *Device::mInstance = NULL;
Device &Device::GetInstance()
{
	if (mInstance == NULL)
	{
		mInstance = NEW Device;
	}
	return *mInstance;
}

void Device::Destroy()
{
	delete mInstance;
	mInstance = NULL;
}

Device::Device()
{
	m_pDevImpl = NEW DeviceImpl;
}

Device::~Device()
{
	if (NULL != m_pDevImpl)
	{
		delete m_pDevImpl;
		m_pDevImpl = NULL;
	}
}

void Device::SetReconnect(fDisConnect cbDisConnect, fHaveReConnect cbAutoConnect, fSubDisConnect cbSubDisConnect, LDWORD dwUser)
{
	if (m_pDevImpl == NULL)
	{
		return ;
	}
	return m_pDevImpl->SetReconnect(cbDisConnect,cbAutoConnect,cbSubDisConnect,dwUser);
}

BOOL Device::Login(const char *pchDVRIP, WORD wDVRPort, const char *pchUserName, const char *pchPassword)
{
	IMPL_IS_OK
	return m_pDevImpl->Login(pchDVRIP,wDVRPort,pchUserName,pchPassword);
}

BOOL Device::Logout()
{
	IMPL_IS_OK
	return m_pDevImpl->Logout();
}

BOOL Device::StartRealPlay(int nChannelID, HWND hWnd)
{
	IMPL_IS_OK
	return m_pDevImpl->StartRealPlay(nChannelID,hWnd);
}

BOOL Device::StopRealPlay()
{
	IMPL_IS_OK
	return m_pDevImpl->StopRealPlay();
}

BOOL Device::StrobeControl(CtrlType emType, int nSelChan)
{
	IMPL_IS_OK
	return m_pDevImpl->StrobeControl(emType,nSelChan);
}

BOOL Device::ManualSnap(CtrlType emType, int nSelChan)
{
	IMPL_IS_OK
	return m_pDevImpl->ManualSnap(emType,nSelChan);
}

BOOL Device::StartRealLoadPicture(int nChannelID, DWORD dwAlarmType, BOOL bNeedPicFile, fAnalyzerDataCallBack cbAnalyzerData, LDWORD dwUser/*, void* Reserved*/)
{
	IMPL_IS_OK
	return m_pDevImpl->StartRealLoadPicture( nChannelID,  dwAlarmType,  bNeedPicFile,  cbAnalyzerData,  dwUser);
}

BOOL Device::StopRealLoadPicture()
{	
	IMPL_IS_OK
	return m_pDevImpl->StopRealLoadPicture();
}

BOOL Device::StartListenEvent(fMessCallBack cbMessage,LDWORD dwUser)
{
	IMPL_IS_OK
	return m_pDevImpl->StartListenEvent(cbMessage,dwUser);
}

BOOL Device::StopListenEvent()
{
	IMPL_IS_OK
	return m_pDevImpl->StopListenEvent();
}

BOOL Device::SetStrobe(NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,void* szInBuffer, DWORD dwOutBufferSize)
{
	IMPL_IS_OK
	return m_pDevImpl->SetStrobe(emCfgOpType,  nChannelID, szInBuffer,  dwOutBufferSize);
}

BOOL Device::GetStrobe(NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,void* szOutBuffer, DWORD dwInBufferSize)
{
	IMPL_IS_OK
	return m_pDevImpl->GetStrobe(emCfgOpType,  nChannelID, szOutBuffer,  dwInBufferSize);
}

BOOL Device::SetLatticeScreen(CtrlType emType, void* pInBuf)
{
	IMPL_IS_OK
	return m_pDevImpl->SetLatticeScreen(emType,pInBuf);
}

BOOL Device::StartImportList(char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, LDWORD dwUserData)
{
	IMPL_IS_OK
	return m_pDevImpl->StartImportList( szInBuf,  nInBufLen,  cbTransFile, dwUserData);
}

BOOL Device::StartExportList(char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, LDWORD dwUserData)
{
	IMPL_IS_OK
	return m_pDevImpl->StartExportList( szInBuf,  nInBufLen,  cbTransFile,  dwUserData);
}

BOOL Device::StopImportList()
{
	IMPL_IS_OK
	return m_pDevImpl->StopImportList();
}

BOOL Device::StopExportList()
{
	IMPL_IS_OK
	return m_pDevImpl->StopExportList();
}

BOOL Device::StartTalk(pfAudioDataCallBack pfcb, LDWORD dwUser)
{
	IMPL_IS_OK
	return m_pDevImpl->StartTalk(pfcb,dwUser);
}

BOOL Device::StopTalk()
{
	IMPL_IS_OK
	return m_pDevImpl->StopTalk();
}

BOOL Device::AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag)
{
	IMPL_IS_OK
	return m_pDevImpl->AudioData(lTalkHandle, pDataBuf, dwBufSize, byAudioFlag);
}

BOOL Device::GetLoginState()
{
	IMPL_IS_OK
	return m_pDevImpl->GetLoginState();
}

int Device::GetChanNum()
{
	IMPL_IS_OK
	return m_pDevImpl->GetChanNum();
}

BOOL Device::SetParkControlInfo(void* pInBuf, void* pOutBuf)
{
	IMPL_IS_OK
	return m_pDevImpl->SetParkControlInfo(pInBuf, pOutBuf);
}

EM_DEVICE_ERR Device::GetDeviceErr()
{
	if (m_pDevImpl == NULL)
	{
		return EM_UNKNOWN_ERR;
	}
	return m_pDevImpl->GetDeviceErr();
}