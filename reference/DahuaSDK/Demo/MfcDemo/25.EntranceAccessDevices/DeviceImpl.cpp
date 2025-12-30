#include "stdafx.h"
#include "DeviceImpl.h"

DeviceImpl::DeviceImpl(void)
{
	m_lLoginID = 0;
	m_lPlayID = 0;
	m_lRealLoadPicHandle = 0;
	m_nTransmitImportHandle = 0;
	m_nTransmitExportHandle = 0;
	m_hTalkHandle = 0;
	m_emDevErr = EM_UNKNOWN_ERR;
	memset(&stunetDevInfo,0,sizeof(stunetDevInfo));
}

DeviceImpl::~DeviceImpl(void)
{
	CLIENT_Cleanup();
}

void DeviceImpl::SetReconnect(fDisConnect cbDisConnect, fHaveReConnect cbAutoConnect, fSubDisConnect cbSubDisConnect, LDWORD dwUser)
{
// 	CLIENT_Init(&DevDisConnect, (LDWORD)this);
// 	CLIENT_SetAutoReconnect(&DevReConnect, (LDWORD)this);
// 	CLIENT_SetSubconnCallBack(&DevSubDisConnect,(LDWORD)this);

	CLIENT_Init(cbDisConnect, (LDWORD)dwUser);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(cbAutoConnect, (LDWORD)dwUser);
	CLIENT_SetSubconnCallBack(cbSubDisConnect,(LDWORD)dwUser);

	NET_PARAM netParam = {0};
	netParam.nGetConnInfoTime = 3000; 
	netParam.nPicBufSize = 5 * 1024 * 1024; // set picture buffer: 5M
	CLIENT_SetNetworkParam(&netParam);
}

BOOL DeviceImpl::Login(const char *pchDVRIP, WORD wDVRPort, const char *pchUserName, const char *pchPassword)
{
	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, pchDVRIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, 		pchPassword, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, pchUserName, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = wDVRPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	m_lLoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (m_lLoginID == 0)
	{
		m_emDevErr = EM_LOGIN_FAIL;
		return FALSE;
	}
	stunetDevInfo = stOutparam.stuDeviceInfo;
	return TRUE;
}

BOOL DeviceImpl::Logout()
{
	BOOL bret = CLIENT_Logout(m_lLoginID);
	if (bret)
	{
		m_lLoginID = 0;
		m_lPlayID = 0;
		m_lRealLoadPicHandle = 0;
		m_nTransmitImportHandle = 0;
		m_nTransmitExportHandle = 0;
		m_hTalkHandle = 0;
		m_emDevErr = EM_UNKNOWN_ERR;
	}
	return bret;
}

BOOL DeviceImpl::StartRealPlay(int nChannelID, HWND hWnd)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	m_lPlayID = CLIENT_RealPlayEx(m_lLoginID, nChannelID, hWnd);
	if (0 == m_lPlayID)
	{
		m_emDevErr = EM_PLAY_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::StopRealPlay()
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	if (0 == m_lPlayID)
	{
		return FALSE;
	}
	else
	{
		CLIENT_StopRealPlayEx(m_lPlayID);
		m_lPlayID = 0;
		return TRUE;
	}
}

BOOL DeviceImpl::StrobeControl(CtrlType emType, int nSelChan)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	NET_CTRL_OPEN_STROBE stuOpenStrobe = {sizeof(NET_CTRL_OPEN_STROBE)};
	stuOpenStrobe.nChannelId = nSelChan;
	BOOL bret = CLIENT_ControlDeviceEx(m_lLoginID, emType, &stuOpenStrobe);
	if (!bret)
	{
		m_emDevErr = EM_OPENSTROBE_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::ManualSnap(CtrlType emType, int nSelChan)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
// 	if (0 == m_lRealLoadPicHandle)
// 	{
// 		m_emDevErr = EM_WITHOUT_SCRIBEPIC;
// 		return FALSE;
// 	}
	MANUAL_SNAP_PARAMETER stuSanpParam = {0};
	stuSanpParam.nChannel = nSelChan;	
	CString strSnapSeq = "1213"; // if the device supports snap sequence, it will return this value in szManualSnapNo of DEV_EVENT_TRAFFIC_MANUALSNAP_INFO 
	memcpy(stuSanpParam.bySequence, strSnapSeq, strSnapSeq.GetLength());
	BOOL bret = CLIENT_ControlDeviceEx(m_lLoginID, emType, &stuSanpParam);
	if (!bret)
	{
		m_emDevErr = EM_SNAP_FAIL;
		return FALSE;
	} 
	return TRUE;
}

BOOL DeviceImpl::StartRealLoadPicture(int nChannelID, DWORD dwAlarmType, BOOL bNeedPicFile, fAnalyzerDataCallBack cbAnalyzerData, LDWORD dwUser/*, void* Reserved*/)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	NET_RESERVED_COMMON stReservedCommon = {0};
	stReservedCommon.dwStructSize = sizeof(NET_RESERVED_COMMON);

	ReservedPara stReserved;
	stReserved.dwType = RESERVED_TYPE_FOR_COMMON;
	stReserved.pData = &stReservedCommon;	
	for (int i = 0; i <= 6; i++)
	{
		stReservedCommon.dwSnapFlagMask += 1 << i;
	}

	m_lRealLoadPicHandle = CLIENT_RealLoadPictureEx(m_lLoginID, nChannelID, dwAlarmType, bNeedPicFile, cbAnalyzerData, dwUser, (void*)&stReserved);
	if (m_lRealLoadPicHandle == 0)
	{
		m_emDevErr = EM_SCRIBEPIC_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::StopRealLoadPicture()
{	
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	if (m_lRealLoadPicHandle != 0)
	{
		BOOL bret = CLIENT_StopLoadPic(m_lRealLoadPicHandle);
		m_lRealLoadPicHandle = 0;
	}
	return TRUE;
}

BOOL DeviceImpl::StartListenEvent(fMessCallBack cbMessage,LDWORD dwUser)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	CLIENT_SetDVRMessCallBack(cbMessage, (LDWORD)dwUser);
	BOOL bRet = CLIENT_StartListenEx(m_lLoginID);
	if (!bRet)
	{
		m_emDevErr = EM_LISTEN_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::StopListenEvent()
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	return CLIENT_StopListen(m_lLoginID);
}

BOOL DeviceImpl::SetStrobe(NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,void* szInBuffer, DWORD dwOutBufferSize)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	BOOL bret = CLIENT_SetConfig(m_lLoginID, emCfgOpType, nChannelID, szInBuffer,dwOutBufferSize, 5000);
	if (!bret)
	{
		m_emDevErr = EM_SET_STROBE_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::GetStrobe(NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,void* szOutBuffer, DWORD dwInBufferSize)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	BOOL bret = CLIENT_GetConfig(m_lLoginID, emCfgOpType, nChannelID, szOutBuffer,dwInBufferSize, 5000);
	if (!bret)
	{
		m_emDevErr = EM_GET_STROBE_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::SetLatticeScreen(CtrlType emType, void* pInBuf)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	BOOL bret = CLIENT_ControlDeviceEx(m_lLoginID, emType, pInBuf, NULL, 3000);
	if (!bret)
	{
		m_emDevErr = EM_SET_LATTICESCREEN_FAIL;
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::StartImportList(char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, LDWORD dwUserData)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	m_nTransmitImportHandle = CLIENT_FileTransmit(m_lLoginID,DH_DEV_BLACKWHITETRANS_START,szInBuf,nInBufLen,cbTransFile,dwUserData,5000);
	if (m_nTransmitImportHandle != 0)
	{
		LLONG nRet = CLIENT_FileTransmit(m_lLoginID,DH_DEV_BLACKWHITETRANS_SEND,(char*)&m_nTransmitImportHandle,sizeof(LLONG),cbTransFile,dwUserData,5000);
		if (nRet == 0)
		{
			m_emDevErr = EM_IMPORTLIST_FAIL;
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		m_emDevErr = EM_IMPORTLIST_FAIL;
		return FALSE;
	}
}

BOOL DeviceImpl::StartExportList(char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, LDWORD dwUserData)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	m_nTransmitExportHandle = CLIENT_FileTransmit(m_lLoginID,DH_DEV_BLACKWHITE_LOAD,szInBuf,nInBufLen,cbTransFile,dwUserData,5000);
	if (m_nTransmitExportHandle == 0)
	{
		m_emDevErr = EM_EXPORTLIST_FAIL;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL DeviceImpl::StopImportList()
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	if (0 == m_nTransmitImportHandle)
	{
		m_emDevErr = EM_IMPORTLIST_FAIL;
		return FALSE;
	}
	CLIENT_FileTransmit(m_lLoginID,DH_DEV_BLACKWHITETRANS_STOP,(char*)&m_nTransmitImportHandle,sizeof(LLONG),NULL,NULL,5000);
	m_nTransmitImportHandle = 0;
	return TRUE;
}

BOOL DeviceImpl::StopExportList()
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	if (0 == m_nTransmitExportHandle)
	{
		m_emDevErr = EM_EXPORTLIST_FAIL;
		return FALSE;
	}
	CLIENT_FileTransmit(m_lLoginID,DH_DEV_BLACKWHITE_LOAD_STOP,(char*)&m_nTransmitExportHandle,sizeof(LLONG),NULL,NULL,5000);
	m_nTransmitExportHandle = 0;
	return TRUE;
}

BOOL DeviceImpl::StartTalk(pfAudioDataCallBack pfcb, LDWORD dwUser)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}

	DHDEV_TALKDECODE_INFO  m_curTalkMode;
	memset(&m_curTalkMode,0,sizeof(m_curTalkMode));
	m_curTalkMode.encodeType = DH_TALK_PCM;
	m_curTalkMode.dwSampleRate = 16000;
	m_curTalkMode.nAudioBit = 16;
	BOOL bret = CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_ENCODE_TYPE, &m_curTalkMode);
	if (!bret)
	{
		m_emDevErr = EM_SET_ENCODE_FAIL;
		return FALSE;
	}

	NET_TALK_TRANSFER_PARAM stTransfer = {sizeof(NET_TALK_TRANSFER_PARAM)};
	stTransfer.bTransfer = 0;
	bret = CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_TRANSFER_MODE, &stTransfer);
	if (!bret)
	{
		m_emDevErr = EM_SET_TRANSFERMODE_FAIL;
		return FALSE;
	}

	m_hTalkHandle = CLIENT_StartTalkEx(m_lLoginID, pfcb, dwUser);
	if(0 != m_hTalkHandle)
	{
		BOOL bSuccess = CLIENT_RecordStart();
		if(bSuccess)
		{
			return TRUE;
		}
		else
		{
			CLIENT_StopTalkEx(m_hTalkHandle);
			m_hTalkHandle = 0;
			m_emDevErr = EM_START_RECORD_FAIL;
			return FALSE;
		}
	}
	else
	{
		m_emDevErr = EM_START_TALK_FAIL;
		return FALSE;
	}
}

BOOL DeviceImpl::StopTalk()
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}
	if (0 == m_hTalkHandle)
	{
		return FALSE;
	}

	CLIENT_RecordStop();

	BOOL bret = CLIENT_StopTalkEx(m_hTalkHandle);
	if(!bret)
	{
		m_emDevErr = EM_STOP_TALK_FAIL;
		return FALSE;
	}
	m_hTalkHandle = 0;
	return TRUE;
}

BOOL DeviceImpl::AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag)
{
	if(m_hTalkHandle == lTalkHandle)
	{
		if(0 == byAudioFlag)
		{
			long lSendLen = CLIENT_TalkSendData(lTalkHandle, pDataBuf, dwBufSize);
			if(lSendLen != (long)dwBufSize)
			{
			}
		}
		else if(1 == byAudioFlag)
		{
			CLIENT_AudioDec(pDataBuf, dwBufSize);			
		}
	}
	return TRUE;
}

BOOL DeviceImpl::GetLoginState()
{
	if (m_lLoginID == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

int DeviceImpl::GetChanNum()
{
	return stunetDevInfo.nChanNum;
}

BOOL DeviceImpl::SetParkControlInfo(void* pInBuf, void* pOutBuf)
{

	if (0 != m_lLoginID)
	{
		BOOL bRet = CLIENT_ControlDeviceEx(m_lLoginID, DH_CTRL_SET_PARK_CONTROL_INFO, pInBuf, pOutBuf);
		if (bRet)
		{
			return TRUE;
		} 
	}
	return FALSE;
}

EM_DEVICE_ERR DeviceImpl::GetDeviceErr()
{
	return m_emDevErr;
}