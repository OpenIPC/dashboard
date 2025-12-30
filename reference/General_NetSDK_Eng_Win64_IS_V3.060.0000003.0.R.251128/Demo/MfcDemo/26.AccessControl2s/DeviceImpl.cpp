#include "stdafx.h"
#include "DeviceImpl.h"
#include "Utility.h"

#define SDK_API_WAIT			(5000)				// delay time of sdk api in msec

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
	m_hUpgradeId = NULL;
	m_CardRecFindId = NULL;
	m_AlarmFindId = NULL;
	memset(&m_stuNetwork,0,sizeof(m_stuNetwork));
	m_LogId = NULL;
	m_UserFindId = NULL;
	m_CardFindId = NULL;
	m_FaceFindId = NULL;
	m_RealLoadPicHandle =0;
}

DeviceImpl::~DeviceImpl(void)
{
	CLIENT_Cleanup();
}

void DeviceImpl::SetReconnect(fDisConnect cbDisConnect, fHaveReConnect cbAutoConnect, fSubDisConnect cbSubDisConnect, LDWORD dwUser)
{
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
	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stuIn = {sizeof(stuIn)};
	strncpy(stuIn.szIP, pchDVRIP, 63);
	stuIn.nPort = wDVRPort;
	strncpy(stuIn.szUserName, pchUserName, 63);
	strncpy(stuIn.szPassword, pchPassword, 63);
	stuIn.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;
	stuIn.pCapParam = NULL;
	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stuOut = {sizeof(stuOut)};
	m_lLoginID = CLIENT_LoginWithHighLevelSecurity(&stuIn, &stuOut);
	if (m_lLoginID == 0)
	{
		m_emDevErr = EM_LOGIN_FAIL;
		return FALSE;
	}
	stunetDevInfo = stuOut.stuDeviceInfo;

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

BOOL DeviceImpl::StartListenPicEvent(int nChannelID, DWORD dwAlarmType, fAnalyzerDataCallBack cbAnalyzerData, LDWORD dwUser)
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}

	if (0 != m_RealLoadPicHandle)
	{
		CLIENT_StopLoadPic(m_RealLoadPicHandle);
		m_RealLoadPicHandle = 0;
	}

	LLONG lRealloadHandle = CLIENT_RealLoadPictureEx(m_lLoginID, nChannelID, dwAlarmType, true, cbAnalyzerData, dwUser, NULL);
	if (lRealloadHandle)
	{
		m_RealLoadPicHandle = lRealloadHandle;
		return TRUE;
	}

	m_emDevErr = EM_REALLOADPIC_FAIL;
	return FALSE;
}

BOOL DeviceImpl::StopListenPicEvent()
{
	if (0 == m_lLoginID)
	{
		m_emDevErr = EM_WITHOUT_LOGIN;
		return FALSE;
	}

	if (0 != m_RealLoadPicHandle)
	{
		CLIENT_StopLoadPic(m_RealLoadPicHandle);
		m_RealLoadPicHandle = 0;
	}

	return TRUE;
}

BOOL DeviceImpl::GetNetWork(CString& csIp, CString& csMask, CString& csGateway)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024*4] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_NETWORK,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		return FALSE;
	}

	CFG_NETWORK_INFO stNetworkInfo = {0};
	DWORD dwRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_NETWORK, szJsonBuf, &stNetworkInfo, sizeof(stNetworkInfo), &dwRetLen);
	if (!bRet)
	{
		return FALSE;
	}

	for (int i=0;i<stNetworkInfo.nInterfaceNum;i++)
	{
		if (strcmp(stNetworkInfo.szDefInterface, stNetworkInfo.stuInterfaces[i].szName) == 0)
		{
			if (0 == stNetworkInfo.stuInterfaces[i].szIP[0] 
			|| 0 == stNetworkInfo.stuInterfaces[i].szSubnetMask[0]
			|| 0 == stNetworkInfo.stuInterfaces[i].szDefGateway[0])
			{
				return FALSE;
			}
			else
			{
				m_stuNetwork = stNetworkInfo;
				csIp = stNetworkInfo.stuInterfaces[i].szIP;
				csMask = stNetworkInfo.stuInterfaces[i].szSubnetMask;
				csGateway = stNetworkInfo.stuInterfaces[i].szDefGateway;
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL DeviceImpl::SetNetWork(CString csIp, CString csMask, CString csGateway)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	CFG_NETWORK_INFO& stuNetworkPara = m_stuNetwork;
	for (int i=0;i<stuNetworkPara.nInterfaceNum;i++)
	{
		if (strcmp("eth0", stuNetworkPara.stuInterfaces[i].szName) == 0)
		{
			strncpy(stuNetworkPara.stuInterfaces[i].szIP, csIp, MAX_ADDRESS_LEN-1);
			strncpy(stuNetworkPara.stuInterfaces[i].szSubnetMask, csMask, MAX_ADDRESS_LEN-1);
			strncpy(stuNetworkPara.stuInterfaces[i].szDefGateway, csGateway, MAX_ADDRESS_LEN-1);
		}
	}

	char szJsonBuf[1024*4] = {0};
	BOOL nRet = CLIENT_PacketData(CFG_CMD_NETWORK, &stuNetworkPara, sizeof(CFG_NETWORK_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!nRet)
	{
		return FALSE;
	}

	int nErr = 0, nRestart = 0;
	nRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_NETWORK,-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!nRet)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::GetRegisterCFG(CFG_DVRIP_INFO& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_DVRIP,-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		return FALSE;
	}

	CFG_DVRIP_INFO stDvrIpInfo = {0};
	DWORD dwRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_DVRIP, szJsonBuf, &stDvrIpInfo, sizeof(stDvrIpInfo), &dwRetLen);
	if (!bRet)
	{
		return FALSE;
	}
	else
	{
		stuInfo = stDvrIpInfo;
		return TRUE;
	}
}

BOOL DeviceImpl::SetRegisterCFG(CFG_DVRIP_INFO* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	char szJsonBuf[1024] = {0};
	BOOL nRet = CLIENT_PacketData(CFG_CMD_DVRIP, stuInfo, sizeof(CFG_DVRIP_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!nRet)
	{
		return FALSE;
	}

	int nErr = 0, nRestart = 0;
	nRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_DVRIP,-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!nRet)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::GetDevTime(NET_TIME& DeviceTime)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
 	NET_TIME stuTime;
 	memset(&stuTime, 0, sizeof(NET_TIME));
 	BOOL bRet = CLIENT_QueryDeviceTime(m_lLoginID, &stuTime, SDK_API_WAIT);
 	if (!bRet)
 	{
 		return FALSE;
 	}
 	else
 	{
 		DeviceTime = stuTime;
 		return TRUE;
 	}
}

BOOL DeviceImpl::SetDevTime(NET_TIME* pDeviceTime)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_SetupDeviceTime(m_lLoginID, pDeviceTime);
	if (!bRet)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL DeviceImpl::GetNTP(CFG_NTP_INFO& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0, nRestart = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_NTP,-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_NTP_INFO stuInfoTemp = {sizeof(CFG_NTP_INFO)};  

		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_NTP, szJsonBuf, (void*)&stuInfoTemp, sizeof(stuInfoTemp), &dwRetLen);
		if (bRet)
		{
			stuInfo = stuInfoTemp;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DeviceImpl::SetNTP(CFG_NTP_INFO* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_NTP, stuInfo, sizeof(*stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		return FALSE;
	} 
	else
	{
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_NTP,-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRetSetupCfg)
		{

			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL DeviceImpl::GetDST(AV_CFG_Locales& stuInfo/*DHDEV_DST_CFG& stuInfo*/)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	AV_CFG_Locales m_stuInfo = {sizeof(m_stuInfo)};
	m_stuInfo.stuDstStart.nStructSize = sizeof(m_stuInfo.stuDstStart);
	m_stuInfo.stuDstEnd.nStructSize = sizeof(m_stuInfo.stuDstEnd);

	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_LOCALS, -1, szJsonBuf, 1024*40, &nerror, 3000);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_LOCALS, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			stuInfo = m_stuInfo;
			return TRUE;
		}
	}
	else
	{			
		return FALSE;
	}
}

BOOL DeviceImpl::SetDST(AV_CFG_Locales* stuInfo/*DHDEV_DST_CFG* stuInfo*/)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_LOCALS, stuInfo, sizeof(CFG_ACCESS_TIMESCHEDULE_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		return FALSE;
	} 
	else
	{		
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_LOCALS, -1, szJsonBuf, 1024*40, &nerror, &nrestart, 5000);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL DeviceImpl::GetCapability(CString& csCap)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_AC_CAPS stuIn = {sizeof(stuIn)};
	NET_OUT_AC_CAPS stuOut = {sizeof(stuOut)};
	BOOL bRet = CLIENT_GetDevCaps(m_lLoginID, NET_ACCESSCONTROL_CAPS,&stuIn, &stuOut,5000);
	if (bRet)
	{
		CString csCapTemp;
		CString csCaps;
		GetCaps(&stuOut.stuACCaps, csCaps);
		csCapTemp += csCaps;
		CString csUserCaps;
		GetUserCaps(&stuOut.stuUserCaps, csUserCaps);
		csCapTemp += csUserCaps;
		CString csCardCaps;
		GetCardCaps(&stuOut.stuCardCaps, csCardCaps);
		csCapTemp += csCardCaps;
		CString csFingerprintCaps;
		GetFingerprintCaps(&stuOut.stuFingerprintCaps, csFingerprintCaps);
		csCapTemp += csFingerprintCaps;
		CString csFaceCaps;
		GetFaceCaps(&stuOut.stuFaceCaps, csFaceCaps);
		csCapTemp += csFaceCaps;
		csCap = csCapTemp;
	}


/*	CString csCapTemp;
	CString csLogService;
	if (GetLogServiceCap(csLogService))
	{
		csCapTemp += csLogService;
	}
	CString csRecordSetFinder;
	if (GetRecordSetFinderCap(csRecordSetFinder))
	{
		csCapTemp += csRecordSetFinder;
	}
	CString csAccessControlCap;
	int nCount;
	if (GetAccessControlCap(csAccessControlCap, nCount))
	{
		csCapTemp += csAccessControlCap;
	}
	csCap += csCapTemp*/;
	return TRUE;
}

BOOL DeviceImpl::GetCaps(NET_AC_CAPS* stuACCaps, CString& csCaps)
{
	CString csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("Channels"), stuACCaps->nChannels); csCaps += csCapTemp;
	csCapTemp.Format("%s = %s\r\n", ConvertString("IsSupportAlarmRecord"), stuACCaps->bSupAccessControlAlarmRecord ? ConvertString("Yes") : ConvertString("No")); csCaps += csCapTemp;
	if (stuACCaps->nCustomPasswordEncryption == 0)
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("PasswordEncryptionType"), ConvertString("Plaintext")); csCaps += csCapTemp;
	}
	else
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("PasswordEncryptionType"), ConvertString("MD5")); csCaps += csCapTemp;
	}
	if (stuACCaps->nSupportFingerPrint == 0)
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("SupportFingerPrint"), ConvertString("Unknown")); csCaps += csCapTemp;
	} 
	else if(stuACCaps->nSupportFingerPrint == 1)
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("SupportFingerPrint"), ConvertString("NotSupport")); csCaps += csCapTemp;
	}
	else if(stuACCaps->nSupportFingerPrint == 2)
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("SupportFingerPrint"), ConvertString("Support")); csCaps += csCapTemp;
	}
	csCapTemp.Format("%s = %s\r\n", ConvertString("IsSupportCardAuth"), stuACCaps->bHasCardAuth ? ConvertString("Yes") : ConvertString("No")); csCaps += csCapTemp;
	csCapTemp.Format("%s = %s\r\n", ConvertString("IsSupportFaceAuth"), stuACCaps->bHasFaceAuth ? ConvertString("Yes") : ConvertString("No")); csCaps += csCapTemp;
	csCapTemp.Format("%s = %s\r\n", ConvertString("IsOnlySingleDoorAuth"), stuACCaps->bOnlySingleDoorAuth ? ConvertString("Yes") : ConvertString("No")); csCaps += csCapTemp;
	csCapTemp.Format("%s = %s\r\n", ConvertString("IsSupportAsynAuth"), stuACCaps->bAsynAuth ? ConvertString("Yes") : ConvertString("No")); csCaps += csCapTemp;
	csCapTemp.Format("%s = %s\r\n", ConvertString("IsSupportUserlsoLate"), stuACCaps->bUserlsoLate ? ConvertString("Yes") : ConvertString("No")); csCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("MaxInsertRate"), stuACCaps->nMaxInsertRate); csCaps += csCapTemp;
	
	if (stuACCaps->stuSpecialDaysSchedule.bSupport)
	{
		csCapTemp.Format("%s = %d\r\n", ConvertString("MaxSpecialDaysSchedules"), stuACCaps->stuSpecialDaysSchedule.nMaxSpecialDaysSchedules); csCaps += csCapTemp;
		csCapTemp.Format("%s = %d\r\n", ConvertString("MaxTimePeriodsPerDay"), stuACCaps->stuSpecialDaysSchedule.nMaxTimePeriodsPerDay); csCaps += csCapTemp;
		csCapTemp.Format("%s = %d\r\n", ConvertString("MaxSpecialDayGroups"), stuACCaps->stuSpecialDaysSchedule.nMaxSpecialDayGroups); csCaps += csCapTemp;
		csCapTemp.Format("%s = %d\r\n", ConvertString("MaxDaysInSpecialDayGroup"), stuACCaps->stuSpecialDaysSchedule.nMaxDaysInSpecialDayGroup); csCaps += csCapTemp;
	}
	csCaps += "\r\n";

	return TRUE;
}

BOOL DeviceImpl::GetUserCaps(NET_ACCESS_USER_CAPS* stuUserCaps, CString& csUserCaps)
{
	CString csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("UserMaxInsertRate"), stuUserCaps->nMaxInsertRate); csUserCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("UserMaxUsers"), stuUserCaps->nMaxUsers); csUserCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("UserMaxFingerPrintsPerUser"), stuUserCaps->nMaxFingerPrintsPerUser); csUserCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("UserMaxCardsPerUser"), stuUserCaps->nMaxCardsPerUser); csUserCaps += csCapTemp;

	csUserCaps += "\r\n";
	return TRUE;
}

BOOL DeviceImpl::GetCardCaps(NET_ACCESS_CARD_CAPS* stuCardCaps, CString& csCardCaps)
{
	CString csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("CardMaxInsertRate"), stuCardCaps->nMaxInsertRate); csCardCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("CardMaxCards"), stuCardCaps->nMaxCards); csCardCaps += csCapTemp;
	csCardCaps += "\r\n";
	return TRUE;
}

BOOL DeviceImpl::GetFingerprintCaps(NET_ACCESS_FINGERPRINT_CAPS* stuFingerprintCaps, CString& csFingerprintCaps)
{
	CString csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FingerprintMaxInsertRate"), stuFingerprintCaps->nMaxInsertRate); csFingerprintCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FingerprintMaxFingerprintSize"), stuFingerprintCaps->nMaxFingerprintSize); csFingerprintCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FingerprintMaxFingerprint"), stuFingerprintCaps->nMaxFingerprint); csFingerprintCaps += csCapTemp;
	csFingerprintCaps += "\r\n";
	return TRUE;
}

BOOL DeviceImpl::GetFaceCaps(NET_ACCESS_FACE_CAPS* stuFaceCaps, CString& csFaceCaps)
{
	CString csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FaceMaxInsertRate"), stuFaceCaps->nMaxInsertRate); csFaceCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FaceMaxFace"), stuFaceCaps->nMaxFace); csFaceCaps += csCapTemp;
	if (stuFaceCaps->nRecognitionType == 0)
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("TargRecognitionType"), ConvertString("WhiteLight")); csFaceCaps += csCapTemp;
	}
	else
	{
		csCapTemp.Format("%s = %s\r\n", ConvertString("TargRecognitionType"), ConvertString("InfraRed")); csFaceCaps += csCapTemp;
	}
	csCapTemp.Format("%s = %d\r\n", ConvertString("FaceMinPhotoSize"), stuFaceCaps->nMinPhotoSize); csFaceCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FaceMaxPhotoSize"), stuFaceCaps->nMaxPhotoSize); csFaceCaps += csCapTemp;
	csCapTemp.Format("%s = %d\r\n", ConvertString("FaceMaxGetPhotoNumber"), stuFaceCaps->nMaxGetPhotoNumber); csFaceCaps += csCapTemp;
	csCapTemp.Format("%s = %s\r\n", ConvertString("FaceIsSupportGetPhoto"), stuFaceCaps->bIsSupportGetPhoto ? ConvertString("Yes") : ConvertString("No")); csFaceCaps += csCapTemp;
	csFaceCaps += "\r\n";
	return TRUE;
}

BOOL DeviceImpl::GetVersion(CString& csVer)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	int nRet = 0;
	DHDEV_VERSION_INFO stuInfo = {0};
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_SOFTWARE, (char*)&stuInfo, sizeof(stuInfo), &nRet, SDK_API_WAIT);
	if (bRet)
	{
		CString csVerTemp;
		csVerTemp.Format("%s: %s\r\n%s: %s\r\n%s: %04d-%02d-%02d\r\n",
					 ConvertString("Serial"),
					 stuInfo.szDevSerialNo,
					 ConvertString("SoftwareVersion"),
					 stuInfo.szSoftWareVersion,
					 ConvertString("ReleaseTime"),
					 ((stuInfo.dwSoftwareBuildDate>>16) & 0xffff),
					 ((stuInfo.dwSoftwareBuildDate>>8) & 0xff),
					 (stuInfo.dwSoftwareBuildDate & 0xff));
		csVer = csVerTemp;
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetMAC(CString& csMAC)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	int nRet = 0;
	DHDEV_NETINTERFACE_INFO stuInfo[DH_MAX_NETINTERFACE_NUM] = {0};
	for (int i = 0; i < DH_MAX_NETINTERFACE_NUM; i++)
	{
		stuInfo[i].dwSize = sizeof(DHDEV_NETINTERFACE_INFO);
	}
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_NETINTERFACE, (char*)&stuInfo, DH_MAX_NETINTERFACE_NUM * sizeof(stuInfo), &nRet, SDK_API_WAIT);
	if (bRet)
	{
		CString csMACTemp;
		csMACTemp.Format("%s: %s\r\n",
			ConvertString("MAC"),
			stuInfo[0].szMAC);
		csMAC = csMACTemp;
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetMCUVer(CString& csMCUVer)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_SYSTEM_INFO stuIn = {sizeof(stuIn)};
	NET_OUT_SYSTEM_INFO stuOut = {sizeof(stuOut)};
	BOOL bRet = CLIENT_QueryDevInfo(m_lLoginID, NET_QUERY_SYSTEM_INFO, &stuIn, &stuOut);
	if (bRet)
	{
		CString csMCUVerTemp;
		csMCUVerTemp.Format("%s: %s\r\n",
			ConvertString("MCUVersion"),
			stuOut.szMCUVersion[0]);
		csMCUVer = csMCUVerTemp;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetAccessCount(int& nCount)
{
	CString csAccessControlCap;
	int nCountTemp;
	if (GetAccessControlCap(csAccessControlCap, nCountTemp))
	{
		nCount = nCountTemp;
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::DevReboot()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_REBOOT, NULL, 3000);
	if (!bRet)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::DevReset()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_RESET_SYSTEM stind = {sizeof(stind)};
	NET_OUT_RESET_SYSTEM stoutd = {sizeof(stoutd)};
	BOOL bRet = CLIENT_ResetSystem(m_lLoginID,&stind, &stoutd ,5000);//可以重置一体机
	if (!bRet)
	{
		DWORD nparam = DH_RESTORE_ALL;
		BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RESTOREDEFAULT, (void*)&nparam, 3000);//可以重置一体机和控制器
		if (!bRet)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL DeviceImpl::ModifyPassword(CString strName, CString strOldPassword, CString strNewPassword)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	USER_INFO_NEW stuOldInfo = {sizeof(stuOldInfo)};
	strncpy(stuOldInfo.name, strName.GetBuffer(0), sizeof(stuOldInfo.name) - 1);
	strncpy(stuOldInfo.passWord, strOldPassword.GetBuffer(0), sizeof(stuOldInfo.passWord) - 1);

	USER_INFO_NEW stuModifiedInfo = {sizeof(stuModifiedInfo)};
	strncpy(stuModifiedInfo.passWord, strNewPassword.GetBuffer(0), sizeof(stuModifiedInfo.passWord)-1);

	BOOL bRet = CLIENT_OperateUserInfoNew(m_lLoginID, 6, &stuModifiedInfo, &stuOldInfo, NULL, 3000);
	if (bRet)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetAutoMatrix(DHDEV_AUTOMT_CFG& stuAutoMT)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	DWORD dRet = 0;
	DHDEV_AUTOMT_CFG stuAUTOMTtmp = {sizeof(stuAUTOMTtmp)};
	BOOL bret = CLIENT_GetDevConfig(m_lLoginID,DH_DEV_AUTOMTCFG,0,&stuAUTOMTtmp,sizeof(stuAUTOMTtmp),&dRet);
	if (bret)
	{
		stuAutoMT = stuAUTOMTtmp;
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::SetAutoMatrix(DHDEV_AUTOMT_CFG* stuAutoMT)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_SetDevConfig(m_lLoginID,DH_DEV_AUTOMTCFG,0,stuAutoMT,sizeof(DHDEV_AUTOMT_CFG));
	if (bret)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::StartUpgrade(char *pchFileName, fUpgradeCallBack cbUpgrade, LDWORD dwUser)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
// 	if (m_hUpgradeId != NULL)
// 	{
// 		return FALSE;
// 	}
	m_hUpgradeId = CLIENT_StartUpgradeEx(m_lLoginID, (EM_UPGRADE_TYPE)0, pchFileName, cbUpgrade, dwUser);
	if (m_hUpgradeId != NULL)
	{
		BOOL bRet = CLIENT_SendUpgrade(m_hUpgradeId);
		if (bRet)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::StopUpgrade()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
// 	if (m_hUpgradeId != NULL)
// 	{
		BOOL bret = CLIENT_StopUpgrade(m_hUpgradeId);
		if (bret)
		{
			m_hUpgradeId = NULL;
			return TRUE;
		}
//	}
	return FALSE;
}

BOOL DeviceImpl::GetTimeSchedule(int nChannelID, CFG_ACCESS_TIMESCHEDULE_INFO& stuTimeSchedule)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	CFG_ACCESS_TIMESCHEDULE_INFO m_stuInfo;
	memset(&m_stuInfo,0,sizeof(m_stuInfo));
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ACCESSTIMESCHEDULE, nChannelID, szJsonBuf, 1024*40, &nerror, 3000);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ACCESSTIMESCHEDULE, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			stuTimeSchedule = m_stuInfo;
			return TRUE;
		}
	}
	else
	{			
		return FALSE;
	}
}

BOOL DeviceImpl::SetTimeSchedule(int nChannelID, CFG_ACCESS_TIMESCHEDULE_INFO* stuTimeSchedule)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ACCESSTIMESCHEDULE, stuTimeSchedule, sizeof(CFG_ACCESS_TIMESCHEDULE_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		return FALSE;
	} 
	else
	{		
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ACCESSTIMESCHEDULE, nChannelID, szJsonBuf, 1024*40, &nerror, &nrestart, 5000);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL DeviceImpl::GetAccessEventCfg(int nChannelID, CFG_ACCESS_EVENT_INFO& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	CFG_ACCESS_EVENT_INFO stuInfoTemp;
	memset(&stuInfoTemp, 0, sizeof(stuInfoTemp));
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ACCESS_EVENT, nChannelID, szJsonBuf, 1024*40, &nerror, 5000);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ACCESS_EVENT, szJsonBuf, (void*)&stuInfoTemp, sizeof(stuInfoTemp), &dwRetLen);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			stuInfo = stuInfoTemp;
			return TRUE;
		}
	}
	else
	{			
		return FALSE;
	}
}

BOOL DeviceImpl::SetAccessEventCfg(int nChannelID, CFG_ACCESS_EVENT_INFO* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ACCESS_EVENT, stuInfo, sizeof(CFG_ACCESS_EVENT_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		return FALSE;
	} 
	else
	{		
		int nerror = 0;
		int nrestart = 0;

		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ACCESS_EVENT, nChannelID, szJsonBuf, 1024*40, &nerror, &nrestart, 5000);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL DeviceImpl::GetSpecialDayGroup(int nId, NET_CFG_ACCESSCTL_SPECIALDAY_GROUP_INFO &stuSpecialdayGroup )
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	if (nId < 0 || nId > 127)
	{
		return FALSE;
	}
	// 获取配置信息(szOutBuffer内存由用户申请释放, 具体见枚举类型说明)
// 	CLIENT_NET_API BOOL CALL_METHOD CLIENT_GetConfig(LLONG lLoginID, NET_EM_CFG_OPERATE_TYPE emCfgOpType, int nChannelID,
// 		void* szOutBuffer, DWORD dwOutBufferSize, int waittime=NET_INTERFACE_DEFAULT_TIMEOUT, void *reserve = NULL);

	NET_CFG_ACCESSCTL_SPECIALDAY_GROUP_INFO stuIn = {sizeof(stuIn)};
	BOOL bret = CLIENT_GetConfig((LLONG)m_lLoginID, NET_EM_CFG_ACCESSCTL_SPECIALDAY_GROUP, nId,&stuSpecialdayGroup, sizeof(NET_CFG_ACCESSCTL_SPECIALDAY_GROUP_INFO));
	if (!bret)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}

BOOL DeviceImpl::SetSpecialDayGroup(int nId, NET_CFG_ACCESSCTL_SPECIALDAY_GROUP_INFO* stuSpecialdayGroup)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	if (nId < 0 || nId > 127)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_SetConfig((LLONG)m_lLoginID, NET_EM_CFG_ACCESSCTL_SPECIALDAY_GROUP, nId, stuSpecialdayGroup, sizeof(NET_CFG_ACCESSCTL_SPECIALDAY_GROUP_INFO));//通道是 假日组序号 0-127
	if (!bret)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}	

BOOL DeviceImpl::GetSpecialDaysSchedule(int nId, NET_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE_INFO &stuSpecialdaySchedule)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	if (nId < 0 || nId > 127)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_GetConfig((LLONG)m_lLoginID, NET_EM_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE,nId,&stuSpecialdaySchedule, sizeof(NET_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE_INFO));
	if (!bret)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
	return TRUE;
}

BOOL DeviceImpl::SetSpecialDaysSchedule(int nId, NET_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE_INFO *stuSpecialdaySchedule)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_SetConfig((LLONG)m_lLoginID, NET_EM_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE, nId, stuSpecialdaySchedule, sizeof(NET_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE_INFO));//通道是 假日组序号 0-127
	if (!bret)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL DeviceImpl::InsertHolidayRecorde(NET_RECORDSET_HOLIDAY* stuInfo, int& nRecNo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_CTRL_RECORDSET_INSERT_PARAM stuParam = {sizeof(stuParam)};
	stuParam.stuCtrlRecordSetInfo.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_IN);
	stuParam.stuCtrlRecordSetInfo.emType = NET_RECORD_ACCESSCTLHOLIDAY;
	stuParam.stuCtrlRecordSetInfo.pBuf = (void*)stuInfo;
	stuParam.stuCtrlRecordSetInfo.nBufLen = sizeof(NET_RECORDSET_HOLIDAY);
	stuParam.stuCtrlRecordSetResult.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_OUT);
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_INSERT, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		nRecNo = stuParam.stuCtrlRecordSetResult.nRecNo;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetHolidayRecorde(NET_RECORDSET_HOLIDAY& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLHOLIDAY;

	NET_RECORDSET_HOLIDAY stuHoliday = {sizeof(stuHoliday)};
	stuHoliday.nRecNo = stuInfo.nRecNo;
	stuParam.pBuf = &stuHoliday;

	int nRet = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_DEV_RECORDSET, (char*)&stuParam, 
		sizeof(stuParam), &nRet, SDK_API_WAIT);
	if (bRet)
	{
		//成功  去显示信息
		stuInfo = stuHoliday;
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::UpdateHolidayRecorde(NET_RECORDSET_HOLIDAY* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLHOLIDAY;
	stuParam.pBuf = (void*)stuInfo;
	stuParam.nBufLen = sizeof(NET_RECORDSET_HOLIDAY);
	int nRet = 0;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_UPDATE, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::RemoveHolidayRecorde(NET_RECORDSET_HOLIDAY* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLHOLIDAY;
	stuParam.pBuf = (void*)&stuInfo->nRecNo;
	stuParam.nBufLen = sizeof(stuInfo->nRecNo);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_REMOVE, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::ClearHolidayRecorde()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLHOLIDAY;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_CLEAR, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::InsertPWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD* stuInfo, int& nRecNo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_CTRL_RECORDSET_INSERT_PARAM stuParam = {sizeof(stuParam)};
	stuParam.stuCtrlRecordSetInfo.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_IN);
	stuParam.stuCtrlRecordSetInfo.emType = NET_RECORD_ACCESSCTLPWD;
	stuParam.stuCtrlRecordSetInfo.pBuf = (void*)stuInfo;
	stuParam.stuCtrlRecordSetInfo.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_PWD);
	stuParam.stuCtrlRecordSetResult.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_OUT);
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_INSERT, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		nRecNo = stuParam.stuCtrlRecordSetResult.nRecNo;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetPWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLPWD;

	NET_RECORDSET_ACCESS_CTL_PWD stuPWD = {sizeof(stuPWD)};
	stuPWD.nRecNo = stuInfo.nRecNo;
	stuParam.pBuf = &stuPWD;

	int nRet = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_DEV_RECORDSET, (char*)&stuParam, 
		sizeof(stuParam), &nRet, SDK_API_WAIT);
	if (bRet)
	{
		stuInfo = stuPWD;
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::UpdatePWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLPWD;
	stuParam.pBuf = (void*)stuInfo;
	stuParam.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_PWD);
	int nRet = 0;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_UPDATE, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::RemovePWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLPWD;
	stuParam.pBuf = (void*)&stuInfo->nRecNo;
	stuParam.nBufLen = sizeof(stuInfo->nRecNo);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_REMOVE, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::ClearPWDRecorde()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLPWD;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_CLEAR, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::InsertCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD* stuInfo, BOOL bEx, int& nRetNo)
{
	NET_CTRL_RECORDSET_INSERT_PARAM stuParam = {sizeof(stuParam)};
	stuParam.stuCtrlRecordSetInfo.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_IN);
	stuParam.stuCtrlRecordSetInfo.emType = NET_RECORD_ACCESSCTLCARD;
	stuParam.stuCtrlRecordSetInfo.pBuf = (void*)stuInfo;
	stuParam.stuCtrlRecordSetInfo.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);

	stuParam.stuCtrlRecordSetResult.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_OUT);

	BOOL bRet = FALSE;
	if (!bEx)
	{
		bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_INSERT, &stuParam, SDK_API_WAIT);
	}
	else 
	{
		bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_INSERTEX, &stuParam, SDK_API_WAIT);
	}
	if (bRet)
	{
		nRetNo = stuParam.stuCtrlRecordSetResult.nRecNo;
	}
	return bRet;
}
BOOL DeviceImpl::GetCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD& stuInfo)
{
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLCARD;

	NET_RECORDSET_ACCESS_CTL_CARD stuCard = {sizeof(stuCard)};
	stuCard.nRecNo = stuInfo.nRecNo;
	stuParam.pBuf = &stuCard;

	int nRet = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_DEV_RECORDSET, (char*)&stuParam, sizeof(stuParam), &nRet, SDK_API_WAIT);
	if (bRet)
	{
		stuInfo = stuCard;
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}
BOOL DeviceImpl::UpdateCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD* stuInfo, BOOL bEx)
{
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLCARD;
	stuParam.pBuf = (void*)stuInfo;
	stuParam.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	BOOL bRet = FALSE;
	if (!bEx)
	{
		bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_UPDATE, &stuParam, SDK_API_WAIT);
	}
	else
	{
		bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_UPDATEEX, &stuParam, SDK_API_WAIT);
	}
	return bRet;
}
BOOL DeviceImpl::RemoveCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD* stuInfo)
{
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLCARD;
	stuParam.pBuf = (void*)&stuInfo->nRecNo;
	stuParam.nBufLen = sizeof(stuInfo->nRecNo);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_REMOVE, &stuParam, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}
BOOL DeviceImpl::ClearCardRecorde()
{
	NET_CTRL_RECORDSET_PARAM stuParam = {sizeof(stuParam)};
	stuParam.emType = NET_RECORD_ACCESSCTLCARD;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_CLEAR, &stuParam, SDK_API_WAIT);
	return bRet;
}

///////////////////
BOOL DeviceImpl::UserInsert(int nNum, NET_ACCESS_USER_INFO* stuUserInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_USER_SERVICE_INSERT stuUserInsertIn = {sizeof(stuUserInsertIn)};
	stuUserInsertIn.nInfoNum = nNum;
	stuUserInsertIn.pUserInfo = stuUserInfo;
	NET_OUT_ACCESS_USER_SERVICE_INSERT stuUserINsertOut = {sizeof(stuUserINsertOut)};
	stuUserINsertOut.nMaxRetNum = nNum;
	stuUserINsertOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessUserService(m_lLoginID, NET_EM_ACCESS_CTL_USER_SERVICE_INSERT, &stuUserInsertIn, &stuUserINsertOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::UserGet(NET_IN_ACCESS_USER_SERVICE_GET *stuUserGetIn, NET_OUT_ACCESS_USER_SERVICE_GET &stuUserGetOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	BOOL bRet = CLIENT_OperateAccessUserService(m_lLoginID, NET_EM_ACCESS_CTL_USER_SERVICE_GET, stuUserGetIn, &stuUserGetOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::UserRemove(NET_IN_ACCESS_USER_SERVICE_REMOVE *stuUserRemoveIn, NET_OUT_ACCESS_USER_SERVICE_REMOVE &stuUserRemoveOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessUserService(m_lLoginID, NET_EM_ACCESS_CTL_USER_SERVICE_REMOVE, stuUserRemoveIn, &stuUserRemoveOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::UserClear()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}  
	NET_IN_ACCESS_USER_SERVICE_CLEAR stuUserClearIn = {sizeof(stuUserClearIn)};
	NET_OUT_ACCESS_USER_SERVICE_CLEAR stuUserClearOut = {sizeof(stuUserClearIn)};
	BOOL bRet = CLIENT_OperateAccessUserService(m_lLoginID, NET_EM_ACCESS_CTL_USER_SERVICE_CLEAR, &stuUserClearIn, &stuUserClearOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::UserStartFind()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_USERINFO_START_FIND stuStartIn = {sizeof(stuStartIn)};
	NET_OUT_USERINFO_START_FIND stuStartOut = {sizeof(stuStartOut)};
	stuStartOut.nTotalCount = 0;
	stuStartOut.nCapNum = 10;
	LLONG UserFindId = CLIENT_StartFindUserInfo(m_lLoginID, &stuStartIn, &stuStartOut,SDK_API_WAIT);
	if (UserFindId != NULL)
	{
		m_UserFindId = UserFindId;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
BOOL DeviceImpl::UserFindNext(int nStartNo, int nMaxNum, NET_ACCESS_USER_INFO* pstuAlarm, int& nRecordNum)
{
	if (0 == m_lLoginID || nMaxNum <= 0 || m_UserFindId == NULL || NULL == pstuAlarm)
	{
		return FALSE;
	}
	NET_IN_USERINFO_DO_FIND stuFindIn = {sizeof(stuFindIn)};
	stuFindIn.nStartNo = nStartNo;
	stuFindIn.nCount = nMaxNum;

	NET_OUT_USERINFO_DO_FIND stuFindOut = {sizeof(stuFindOut)};
	stuFindOut.nMaxNum = nMaxNum;
	stuFindOut.pstuInfo = pstuAlarm;


	if (CLIENT_DoFindUserInfo(m_UserFindId, &stuFindIn, &stuFindOut, SDK_API_WAIT))
	{
		if (stuFindOut.nRetNum > 0)
		{
			nRecordNum = stuFindOut.nRetNum;
			return TRUE;
		}
	}
	return FALSE;
}
BOOL DeviceImpl::UserStopFind()
{
	if (0 == m_lLoginID || m_UserFindId == NULL)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_StopFindUserInfo(m_UserFindId);
	m_UserFindId = NULL;
	return bret;
}

BOOL DeviceImpl::CardInsert(int nNum, NET_ACCESS_CARD_INFO* stuCardInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_CARD_SERVICE_INSERT stuCardInsertIn = {sizeof(stuCardInsertIn)};
	stuCardInsertIn.nInfoNum = nNum;
	stuCardInsertIn.pCardInfo = stuCardInfo;
	NET_OUT_ACCESS_CARD_SERVICE_INSERT stuCardINsertOut = {sizeof(stuCardINsertOut)};
	stuCardINsertOut.nMaxRetNum = nNum;
	stuCardINsertOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessCardService(m_lLoginID, NET_EM_ACCESS_CTL_CARD_SERVICE_INSERT, &stuCardInsertIn, &stuCardINsertOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::CardGet(NET_IN_ACCESS_CARD_SERVICE_GET *stuCardGetIn, NET_OUT_ACCESS_CARD_SERVICE_GET &stuCardGetOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessCardService(m_lLoginID, NET_EM_ACCESS_CTL_CARD_SERVICE_GET, stuCardGetIn, &stuCardGetOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::CardUpdate(int nNum, NET_ACCESS_CARD_INFO* stuCardInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_CARD_SERVICE_UPDATE stuCardUpdateIn = {sizeof(stuCardUpdateIn)};
	stuCardUpdateIn.nInfoNum = nNum;
	stuCardUpdateIn.pCardInfo = stuCardInfo;
	NET_OUT_ACCESS_CARD_SERVICE_UPDATE stuCardUpdateOut = {sizeof(stuCardUpdateOut)};
	stuCardUpdateOut.nMaxRetNum = nNum;
	stuCardUpdateOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessCardService(m_lLoginID, NET_EM_ACCESS_CTL_CARD_SERVICE_UPDATE, &stuCardUpdateIn, &stuCardUpdateOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::CardRemove(NET_IN_ACCESS_CARD_SERVICE_REMOVE *stuCardRemoveIn, NET_OUT_ACCESS_CARD_SERVICE_REMOVE &stuCardRemoveOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessCardService(m_lLoginID, NET_EM_ACCESS_CTL_CARD_SERVICE_REMOVE, stuCardRemoveIn, &stuCardRemoveOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::CardClear()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}  
	NET_IN_ACCESS_CARD_SERVICE_CLEAR stuCardClearIn = {sizeof(stuCardClearIn)};
	NET_OUT_ACCESS_CARD_SERVICE_CLEAR stuCardClearOut = {sizeof(stuCardClearOut)};
	BOOL bRet = CLIENT_OperateAccessCardService(m_lLoginID, NET_EM_ACCESS_CTL_CARD_SERVICE_CLEAR, &stuCardClearIn, &stuCardClearOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::CardStartFind()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_CARDINFO_START_FIND stuStartIn = {sizeof(stuStartIn)};
	NET_OUT_CARDINFO_START_FIND stuStartOut = {sizeof(stuStartOut)};
	stuStartOut.nTotalCount = 0;
	stuStartOut.nCapNum = 10;
	LLONG CardFindId = CLIENT_StartFindCardInfo(m_lLoginID, &stuStartIn, &stuStartOut,SDK_API_WAIT);
	if (CardFindId != NULL)
	{
		m_CardFindId = CardFindId;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
BOOL DeviceImpl::CardFindNext(int nStartNo, int nMaxNum, NET_ACCESS_CARD_INFO* pstuAlarm, int& nRecordNum)
{
	if (0 == m_lLoginID || nMaxNum <= 0 || m_CardFindId == NULL || NULL == pstuAlarm)
	{
		return FALSE;
	}
	NET_IN_CARDINFO_DO_FIND stuFindIn = {sizeof(stuFindIn)};
	stuFindIn.nStartNo = nStartNo;
	stuFindIn.nCount = nMaxNum;

	NET_OUT_CARDINFO_DO_FIND stuFindOut = {sizeof(stuFindOut)};
	stuFindOut.nMaxNum = nMaxNum;
	stuFindOut.pstuInfo = pstuAlarm;


	if (CLIENT_DoFindCardInfo(m_CardFindId, &stuFindIn, &stuFindOut, SDK_API_WAIT))
	{
		if (stuFindOut.nRetNum > 0)
		{
			nRecordNum = stuFindOut.nRetNum;
			return TRUE;
		}
	}
	return FALSE;
}
BOOL DeviceImpl::CardStopFind()
{
	if (0 == m_lLoginID || m_CardFindId == NULL)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_StopFindCardInfo(m_CardFindId);
	m_CardFindId = NULL;
	return bret;
}

BOOL DeviceImpl::FaceInsert(int nNum, NET_ACCESS_FACE_INFO* stuFaceInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_FACE_SERVICE_INSERT stuFaceInsertIn = {sizeof(stuFaceInsertIn)};
	stuFaceInsertIn.nFaceInfoNum = nNum;
	stuFaceInsertIn.pFaceInfo = stuFaceInfo;
	NET_OUT_ACCESS_FACE_SERVICE_INSERT stuFaceINsertOut = {sizeof(stuFaceINsertOut)};
	stuFaceINsertOut.nMaxRetNum = nNum;
	stuFaceINsertOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessFaceService(m_lLoginID, NET_EM_ACCESS_CTL_FACE_SERVICE_INSERT, &stuFaceInsertIn, &stuFaceINsertOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::FaceGet(NET_IN_ACCESS_FACE_SERVICE_GET *stuFaceGetIn, NET_OUT_ACCESS_FACE_SERVICE_GET &stuFaceGetOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessFaceService(m_lLoginID, NET_EM_ACCESS_CTL_FACE_SERVICE_GET, stuFaceGetIn, &stuFaceGetOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::FaceUpdate(int nNum, NET_ACCESS_FACE_INFO* stuFaceInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_FACE_SERVICE_UPDATE stuFaceUpdateIn = {sizeof(stuFaceUpdateIn)};
	stuFaceUpdateIn.nFaceInfoNum = nNum;
	stuFaceUpdateIn.pFaceInfo = stuFaceInfo;
	NET_OUT_ACCESS_FACE_SERVICE_UPDATE stuFaceUpdateOut = {sizeof(stuFaceUpdateOut)};
	stuFaceUpdateOut.nMaxRetNum = nNum;
	stuFaceUpdateOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessFaceService(m_lLoginID, NET_EM_ACCESS_CTL_FACE_SERVICE_UPDATE, &stuFaceUpdateIn, &stuFaceUpdateOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::FaceRemove(NET_IN_ACCESS_FACE_SERVICE_REMOVE *stuFaceRemoveIn, NET_OUT_ACCESS_FACE_SERVICE_REMOVE &stuFaceRemoveOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessFaceService(m_lLoginID, NET_EM_ACCESS_CTL_FACE_SERVICE_REMOVE, stuFaceRemoveIn, &stuFaceRemoveOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::FaceClear()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}  
	NET_IN_ACCESS_FACE_SERVICE_CLEAR stuFaceClearIn = {sizeof(stuFaceClearIn)};
	NET_OUT_ACCESS_FACE_SERVICE_CLEAR stuFaceClearOut = {sizeof(stuFaceClearOut)};
	BOOL bRet = CLIENT_OperateAccessFaceService(m_lLoginID, NET_EM_ACCESS_CTL_FACE_SERVICE_CLEAR, &stuFaceClearIn, &stuFaceClearOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::FaceStartFind()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_FACEINFO_START_FIND stuStartIn = {sizeof(stuStartIn)};
	NET_OUT_ACCESS_FACEINFO_START_FIND stuStartOut = {sizeof(stuStartOut)};
	stuStartOut.nTotalCount = 0;
	stuStartOut.nCapNum = 10;
	LLONG FaceFindId = CLIENT_AccessStartFindFaceInfo(m_lLoginID, &stuStartIn, &stuStartOut,SDK_API_WAIT);
	if (FaceFindId != NULL)
	{
		m_FaceFindId = FaceFindId;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::FaceFindNext(int nStartNo, int nMaxNum, NET_FACEINFO* pstuAlarm, int& nRecordNum)
{
	if (0 == m_lLoginID || nMaxNum <= 0 || m_FaceFindId == NULL || NULL == pstuAlarm)
	{
		return FALSE;
	}
	NET_IN_ACCESS_FACEINFO_DO_FIND stuFindIn = {sizeof(stuFindIn)};
	stuFindIn.nStartNo = nStartNo;
	stuFindIn.nCount = nMaxNum;

	NET_OUT_ACCESS_FACEINFO_DO_FIND stuFindOut = {sizeof(stuFindOut)};
	stuFindOut.nMaxNum = nMaxNum;
	stuFindOut.pstuInfo = pstuAlarm;

	if (CLIENT_AccessDoFindFaceInfo(m_FaceFindId, &stuFindIn, &stuFindOut, SDK_API_WAIT))
	{
		if (stuFindOut.nRetNum > 0)
		{
			nRecordNum = stuFindOut.nRetNum;
			return TRUE;
		}
	}
	return FALSE;
}	

BOOL DeviceImpl::FaceStopFind()
{
	if (0 == m_lLoginID || m_CardFindId == NULL)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_AccessStopFindFaceInfo(m_FaceFindId);
	m_FaceFindId = NULL;
	return bret;
}

BOOL DeviceImpl::FingerprintInsert(int nNum, NET_ACCESS_FINGERPRINT_INFO* stuFingerPrintInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_FINGERPRINT_SERVICE_INSERT stuFingerPrintInsertIn = {sizeof(stuFingerPrintInsertIn)};
	stuFingerPrintInsertIn.nFpNum = nNum;
	stuFingerPrintInsertIn.pFingerPrintInfo = stuFingerPrintInfo;
	NET_OUT_ACCESS_FINGERPRINT_SERVICE_INSERT stuFingerPrintINsertOut = {sizeof(stuFingerPrintINsertOut)};
	stuFingerPrintINsertOut.nMaxRetNum = nNum;
	stuFingerPrintINsertOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessFingerprintService(m_lLoginID, NET_EM_ACCESS_CTL_FINGERPRINT_SERVICE_INSERT, &stuFingerPrintInsertIn, &stuFingerPrintINsertOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::FingerprintGet(NET_IN_ACCESS_FINGERPRINT_SERVICE_GET *stuFingerprintGetIn, NET_OUT_ACCESS_FINGERPRINT_SERVICE_GET &stuFingerprintGetOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessFingerprintService(m_lLoginID, NET_EM_ACCESS_CTL_FINGERPRINT_SERVICE_GET, stuFingerprintGetIn, &stuFingerprintGetOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::FingerprintUpdate(int nNum, NET_ACCESS_FINGERPRINT_INFO* stuFingerPrintInfo, NET_EM_FAILCODE &stuFailCode)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_ACCESS_FINGERPRINT_SERVICE_UPDATE stuFingerprintUpdateIn = {sizeof(stuFingerprintUpdateIn)};
	stuFingerprintUpdateIn.nFpNum = nNum;
	stuFingerprintUpdateIn.pFingerPrintInfo = stuFingerPrintInfo;
	NET_OUT_ACCESS_FINGERPRINT_SERVICE_UPDATE stuFingerprintUpdateOut = {sizeof(stuFingerprintUpdateOut)};
	stuFingerprintUpdateOut.nMaxRetNum = nNum;
	stuFingerprintUpdateOut.pFailCode = &stuFailCode;
	BOOL bRet = CLIENT_OperateAccessFingerprintService(m_lLoginID, NET_EM_ACCESS_CTL_FINGERPRINT_SERVICE_UPDATE, &stuFingerprintUpdateIn, &stuFingerprintUpdateOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::FingerprintRemove(NET_IN_ACCESS_FINGERPRINT_SERVICE_REMOVE *stuFingerprintRemoveIn, NET_OUT_ACCESS_FINGERPRINT_SERVICE_REMOVE &stuFingerprintRemoveOut)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	BOOL bRet = CLIENT_OperateAccessFingerprintService(m_lLoginID, NET_EM_ACCESS_CTL_FINGERPRINT_SERVICE_REMOVE, stuFingerprintRemoveIn, &stuFingerprintRemoveOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}
BOOL DeviceImpl::FingerprintClear()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}  
	NET_IN_ACCESS_FINGERPRINT_SERVICE_CLEAR stuFingerprintClearIn = {sizeof(stuFingerprintClearIn)};
	NET_OUT_ACCESS_FINGERPRINT_SERVICE_CLEAR stuFingerprintClearOut = {sizeof(stuFingerprintClearOut)};
	BOOL bRet = CLIENT_OperateAccessFingerprintService(m_lLoginID, NET_EM_ACCESS_CTL_FINGERPRINT_SERVICE_CLEAR, &stuFingerprintClearIn, &stuFingerprintClearOut, SDK_API_WAIT);
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

//////////////////

BOOL DeviceImpl::GetOpenDoorGroup(int nChannelID, CFG_OPEN_DOOR_GROUP_INFO& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	CFG_OPEN_DOOR_GROUP_INFO stuInfoTemp;
	memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_OPEN_DOOR_GROUP, nChannelID, szJsonBuf, 1024*40, &nerror, 5000);
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_OPEN_DOOR_GROUP, szJsonBuf, (void*)&stuInfoTemp, sizeof(stuInfoTemp), &dwRetLen);
		if (bRet)
		{
			stuInfo = stuInfoTemp;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DeviceImpl::SetOpenDoorGroup(int nChannelID, CFG_OPEN_DOOR_GROUP_INFO* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_OPEN_DOOR_GROUP, stuInfo, sizeof(CFG_OPEN_DOOR_GROUP_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		return FALSE;
	} 
	else
	{		
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_OPEN_DOOR_GROUP, nChannelID, szJsonBuf, 1024*40, &nerror, &nrestart, 5000);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL DeviceImpl::GetAccessGeneralInfo(CFG_ACCESS_GENERAL_INFO& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	CFG_ACCESS_GENERAL_INFO stuInfoTemp;
	memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ACCESS_GENERAL, -1, szJsonBuf, 1024*40, &nerror, 5000);
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ACCESS_GENERAL, szJsonBuf, (void*)&stuInfoTemp, sizeof(stuInfoTemp), &dwRetLen);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			stuInfo = stuInfoTemp;
			return TRUE;
		}
	}
	else
	{			
		return FALSE;
	}
}

BOOL DeviceImpl::SetAccessGeneralInfo(CFG_ACCESS_GENERAL_INFO* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ACCESS_GENERAL, stuInfo, sizeof(CFG_ACCESS_GENERAL_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		return FALSE;
	} 
	else
	{		
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ACCESS_GENERAL, -1, szJsonBuf, 1024*40, &nerror, &nrestart, 5000);
		if (!bRet)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

BOOL DeviceImpl::GetOpenDoorRouteInfo(CFG_OPEN_DOOR_ROUTE_INFO& stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	int nCount = 0;
	GetAccessCount(nCount);
	if (nCount > 0)
	{
		CFG_OPEN_DOOR_ROUTE_INFO stuInfoTemp;
		memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
		char szJsonBuf[1024 * 40] = {0};
		int nerror = 0;
		BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_OPEN_DOOR_ROUTE, -1, szJsonBuf, 1024*40, &nerror, 5000);
		if (bRet)
		{
			DWORD dwRetLen = 0;
			bRet = CLIENT_ParseData(CFG_CMD_OPEN_DOOR_ROUTE, szJsonBuf, (void*)&stuInfoTemp, sizeof(stuInfoTemp), &dwRetLen);
			if (!bRet)
			{
				return FALSE;
			}
			else
			{
				stuInfo = stuInfoTemp;
				return TRUE;
			}
		}
		else
		{			
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::SetOpenDoorRouteInfo(CFG_OPEN_DOOR_ROUTE_INFO* stuInfo)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	int nCount = 0;
	GetAccessCount(nCount);
	BOOL bResult = TRUE;
	for (int i=0;i<nCount;i++)
	{
		char szJsonBuf[1024 * 40] = {0};
		BOOL bRet = CLIENT_PacketData(CFG_CMD_OPEN_DOOR_ROUTE, stuInfo, sizeof(CFG_OPEN_DOOR_ROUTE_INFO), szJsonBuf, sizeof(szJsonBuf));
		if (!bRet)
		{
			bResult = FALSE;
		} 
		else
		{		
			int nerror = 0;
			int nrestart = 0;
			bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_OPEN_DOOR_ROUTE, i, szJsonBuf, 1024*40, &nerror, &nrestart, 5000);
			if (!bRet)
			{
				bResult = FALSE;
			}
		}
	}
	return bResult;
}

BOOL DeviceImpl::CardRecStartFind(SYSTEMTIME stStart, SYSTEMTIME stEnd)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_FIND_RECORD_PARAM stuIn = {sizeof(stuIn)};
	NET_OUT_FIND_RECORD_PARAM stuOut = {sizeof(stuOut)};

	stuIn.emType = NET_RECORD_ACCESSCTLCARDREC_EX;

	FIND_RECORD_ACCESSCTLCARDREC_CONDITION_EX stuCondition = {sizeof(stuCondition)};
	stuCondition.bTimeEnable = TRUE;
	stuCondition.stStartTime.dwYear = stStart.wYear;
	stuCondition.stStartTime.dwMonth = stStart.wMonth;
	stuCondition.stStartTime.dwDay = stStart.wDay;
	stuCondition.stStartTime.dwHour = stStart.wHour;
	stuCondition.stStartTime.dwMinute = stStart.wMinute;
	stuCondition.stStartTime.dwSecond = stStart.wSecond;
	stuCondition.stEndTime.dwYear = stEnd.wYear;
	stuCondition.stEndTime.dwMonth = stEnd.wMonth;
	stuCondition.stEndTime.dwDay = stEnd.wDay;
	stuCondition.stEndTime.dwHour = stEnd.wHour;
	stuCondition.stEndTime.dwMinute = stEnd.wMinute;
	stuCondition.stEndTime.dwSecond = stEnd.wSecond;
	stuIn.pQueryCondition = &stuCondition;
	if (CLIENT_FindRecord(m_lLoginID, &stuIn, &stuOut, SDK_API_WAIT))
	{
		m_CardRecFindId = stuOut.lFindeHandle;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::CardRecFindNext(int nMaxNum, NET_RECORDSET_ACCESS_CTL_CARDREC* pstuCardRec, int& nRecordNum)
{
	if (0 == m_lLoginID || nMaxNum <= 0 || m_CardRecFindId == NULL || NULL == pstuCardRec)
	{
		return FALSE;
	}
	int i = 0, j = 0;
	NET_IN_FIND_NEXT_RECORD_PARAM stuIn = {sizeof(stuIn)};
	stuIn.lFindeHandle = m_CardRecFindId;
	stuIn.nFileCount = nMaxNum;

	NET_OUT_FIND_NEXT_RECORD_PARAM stuOut = {sizeof(stuOut)};
	stuOut.nMaxRecordNum = nMaxNum;
	stuOut.pRecordList = (void*)pstuCardRec;

	if (CLIENT_FindNextRecord(&stuIn, &stuOut, SDK_API_WAIT) >= 0)
	{
		if (stuOut.nRetRecordNum > 0)
		{
			nRecordNum = stuOut.nRetRecordNum;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DeviceImpl::CardRecStopFind()
{
	if (0 == m_lLoginID || m_CardRecFindId == NULL)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_FindRecordClose(m_CardRecFindId);
	if (bret)
	{
		m_CardRecFindId = NULL;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::CardRecQueryCount(int& nCount)
{
	if (0 == m_lLoginID || m_CardRecFindId == NULL)
	{
		return FALSE;
	}
	NET_IN_QUEYT_RECORD_COUNT_PARAM stuIn = {sizeof(stuIn)};
	stuIn.lFindeHandle = m_CardRecFindId;
	NET_OUT_QUEYT_RECORD_COUNT_PARAM stuOut = {sizeof(stuOut)};
	if (CLIENT_QueryRecordCount(&stuIn, &stuOut, SDK_API_WAIT))
	{
		nCount = stuOut.nRecordCount;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::AlarmStartFind()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_FIND_RECORD_PARAM stuIn = {sizeof(stuIn)};
	NET_OUT_FIND_RECORD_PARAM stuOut = {sizeof(stuOut)};

	stuIn.emType = NET_RECORD_ACCESS_ALARMRECORD;//NET_RECORD_ALARMRECORD;
	if (CLIENT_FindRecord(m_lLoginID, &stuIn, &stuOut, SDK_API_WAIT))
	{
		m_AlarmFindId = stuOut.lFindeHandle;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::AlarmFindNext(int nMaxNum, NET_RECORD_ACCESS_ALARMRECORD_INFO* pstuAlarm, int& nRecordNum)
{
	if (0 == m_lLoginID || nMaxNum <= 0 || m_AlarmFindId == NULL || NULL == pstuAlarm)
	{
		return FALSE;
	}
	int i = 0, j = 0;
	NET_IN_FIND_NEXT_RECORD_PARAM stuIn = {sizeof(stuIn)};
	stuIn.lFindeHandle = m_AlarmFindId;
	stuIn.nFileCount = nMaxNum;

	NET_OUT_FIND_NEXT_RECORD_PARAM stuOut = {sizeof(stuOut)};
	stuOut.nMaxRecordNum = nMaxNum;
	stuOut.pRecordList = (void*)pstuAlarm;

	if (CLIENT_FindNextRecord(&stuIn, &stuOut, SDK_API_WAIT) >= 0)
	{
		if (stuOut.nRetRecordNum > 0)
		{
			nRecordNum = stuOut.nRetRecordNum;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DeviceImpl::AlarmStopFind()
{
	if (0 == m_lLoginID || m_AlarmFindId == NULL)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_FindRecordClose(m_AlarmFindId);
	if (bret)
	{
		m_AlarmFindId = NULL;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::AlarmQueryCount(int& nCount)
{
	if (0 == m_lLoginID || m_AlarmFindId == NULL)
	{
		return FALSE;
	}
	NET_IN_QUEYT_RECORD_COUNT_PARAM stuIn = {sizeof(stuIn)};
	stuIn.lFindeHandle = m_AlarmFindId;
	NET_OUT_QUEYT_RECORD_COUNT_PARAM stuOut = {sizeof(stuOut)};
	if (CLIENT_QueryRecordCount(&stuIn, &stuOut, SDK_API_WAIT))
	{
		nCount = stuOut.nRecordCount;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::LogStartFind()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_START_QUERYLOG stuIn = {sizeof(stuIn)};
	NET_OUT_START_QUERYLOG stuOut = {sizeof(stuOut)};
	LLONG lLogID = CLIENT_StartQueryLog(m_lLoginID, &stuIn, &stuOut, SDK_API_WAIT);
	if (lLogID != NULL)
	{
		m_LogId = lLogID;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::LogFindNext(int nMaxNum, NET_LOG_INFO* pstuAlarm, int& nRecordNum)
{
	if (0 == m_lLoginID || nMaxNum <= 0 || m_LogId == NULL || NULL == pstuAlarm)
	{
		return FALSE;
	}
	NET_IN_QUERYNEXTLOG stuIn = {sizeof(stuIn)};
	stuIn.nGetCount = 10;

	NET_OUT_QUERYNEXTLOG stuOut = {sizeof(stuOut)};
	stuOut.nMaxCount = 10;
	stuOut.pstuLogInfo = pstuAlarm;
	if (CLIENT_QueryNextLog(m_LogId, &stuIn, &stuOut, SDK_API_WAIT))
	{
		if (stuOut.nRetCount > 0)
		{
			nRecordNum = stuOut.nRetCount;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DeviceImpl::LogStopFind()
{
	if (0 == m_lLoginID || m_LogId == NULL)
	{
		return FALSE;
	}
	BOOL bret = CLIENT_StopQueryLog(m_LogId);
	return bret;
}

BOOL DeviceImpl::LogQueryCount(int& nCount)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_GETCOUNT_LOG_PARAM stuIn = {sizeof(stuIn)};
	NET_OUT_GETCOUNT_LOG_PARAM stuOut = {sizeof(stuOut)};
	if (CLIENT_QueryDevLogCount(m_lLoginID, &stuIn, &stuOut, SDK_API_WAIT))
	{
		nCount = stuOut.nLogCount;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::OpenDoor(int nChanne)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	CFG_ACCESS_EVENT_INFO stuInfoTemp;
	memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
	BOOL bret = GetAccessEventCfg(nChanne, stuInfoTemp);
	if (bret)
	{
		if (stuInfoTemp.emState != ACCESS_STATE_NORMAL)
		{
			stuInfoTemp.emState = ACCESS_STATE_NORMAL;
			bret = SetAccessEventCfg(nChanne,&stuInfoTemp);
			if (!bret)
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}

	NET_CTRL_ACCESS_OPEN stuParam = {sizeof(stuParam)};
	stuParam.nChannelID = nChanne;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_ACCESS_OPEN, &stuParam, 3000);
	if (bRet)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::CloseDoor(int nChanne)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	CFG_ACCESS_EVENT_INFO stuInfoTemp;
	memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
	BOOL bret = GetAccessEventCfg(nChanne, stuInfoTemp);
	if (bret)
	{
		if (stuInfoTemp.emState != ACCESS_STATE_NORMAL)
		{
			stuInfoTemp.emState = ACCESS_STATE_NORMAL;
			bret = SetAccessEventCfg(nChanne,&stuInfoTemp);
			if (!bret)
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}

	NET_CTRL_ACCESS_CLOSE stuParam = {sizeof(stuParam)};
	stuParam.nChannelID = nChanne;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_ACCESS_CLOSE, &stuParam, 3000);
	if (bRet)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::OpenAlways(int nChanne)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	CFG_ACCESS_EVENT_INFO stuInfoTemp;
	memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
	BOOL bret = GetAccessEventCfg(nChanne, stuInfoTemp);
	if (bret)
	{
		stuInfoTemp.emState = ACCESS_STATE_OPENALWAYS;
		bret = SetAccessEventCfg(nChanne,&stuInfoTemp);
		if (bret)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::CloseAlways(int nChanne)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	CFG_ACCESS_EVENT_INFO stuInfoTemp;
	memset(&stuInfoTemp,0,sizeof(stuInfoTemp));
	BOOL bret = GetAccessEventCfg(nChanne, stuInfoTemp);
	if (bret)
	{
		stuInfoTemp.emState = ACCESS_STATE_CLOSEALWAYS;
		bret = SetAccessEventCfg(nChanne,&stuInfoTemp);
		if (bret)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL DeviceImpl::GetDoorState(int nChanne, EM_NET_DOOR_STATUS_TYPE& emStateType)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_DOOR_STATUS_INFO stuParam = {sizeof(stuParam)};
	stuParam.nChannel = nChanne;
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_DOOR_STATE, (char*)&stuParam, sizeof(stuParam), &nRetLen, 3000);
	if (bRet)
	{
		emStateType = stuParam.emStateType;
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
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



EM_DEVICE_ERR DeviceImpl::GetDeviceErr()
{
	return m_emDevErr;
}

DWORD DeviceImpl::GetLastError()
{
	return CLIENT_GetLastError();
}

BOOL DeviceImpl::GetLogServiceCap(CString& csCap)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szBuff[1024] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_lLoginID, CFG_CAP_CMD_LOG, 0, szBuff, sizeof(szBuff), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_LOG stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_LOG, szBuff, &stuInfo, sizeof(CFG_CAP_LOG), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_LOG))
		{
			CString csLogServiceCap;
			csLogServiceCap.Format("%s = %d\r\n%s = %d\r\n%s = %s\r\n%s = %s\r\n%s = %s\r\n",
								   ConvertString("LogMaxItem"),
								   stuInfo.dwMaxLogItems,
								   ConvertString("MaxPageLogItem"),
								   stuInfo.dwMaxPageItems,
								   ConvertString("IsSupportStartNo"),
								   stuInfo.bSupportStartNo ? ConvertString("Yes") : ConvertString("No"),
								   ConvertString("IsSupportTypeFilter"),
								   stuInfo.bSupportTypeFilter ? ConvertString("Yes") : ConvertString("No"),
								   ConvertString("IsSupportTimeFilter"),
								   stuInfo.bSupportTimeFilter ? ConvertString("Yes") : ConvertString("No"));
			csCap += csLogServiceCap;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}

BOOL DeviceImpl::GetRecordSetFinderCap(CString& csCap)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	char szBuff[1024] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_lLoginID, CFG_CAP_CMD_RECORDFINDER, 0, szBuff, sizeof(szBuff), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_RECORDFINDER_INFO stuCap = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_RECORDFINDER, szBuff, &stuCap, sizeof(stuCap), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_RECORDFINDER_INFO))
		{
			csCap += ConvertString("RecordSetFinder Cap:");
			csCap += "\r\n";

			CString csMaxPageSize;
			csMaxPageSize.Format("%s = %d\r\n",
				ConvertString("MaxPageSize"),
				stuCap.nMaxPageSize);
			csCap += csMaxPageSize;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL DeviceImpl::GetAccessControlCap(CString& csCap, int& nCount)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}

	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_AC_CAPS stuIn = {sizeof(stuIn)};
	NET_OUT_AC_CAPS stuOut = {sizeof(stuOut)};
	BOOL bRet = CLIENT_GetDevCaps(m_lLoginID, NET_ACCESSCONTROL_CAPS,&stuIn, &stuOut,5000);
	if (bRet)
	{
		nCount = stuOut.stuACCaps.nChannels;
	}
	else
	{
		return FALSE;
	}
	return TRUE;

// 	char szBuf[1024] = {0};
// 	int nError = 0;
// 	BOOL bRet = CLIENT_QueryNewSystemInfo(m_lLoginID, CFG_CAP_CMD_ACCESSCONTROLMANAGER, -1, szBuf, sizeof(szBuf), &nError, SDK_API_WAIT);
// 	if (bRet)
// 	{
// 		CFG_CAP_ACCESSCONTROL stuCap = {0};
// 		DWORD dwRet = 0;
// 		bRet = CLIENT_ParseData(CFG_CAP_CMD_ACCESSCONTROLMANAGER, szBuf, &stuCap, sizeof(stuCap), &dwRet);
// 		if (bRet && dwRet == sizeof(CFG_CAP_ACCESSCONTROL))
// 		{
// 			nCount = stuCap.nAccessControlGroups;
// 			csCap += ConvertString("AccessControlManager Cap:");
// 
// 			csCap += "\r\n";
// 			CString csAccessControl;
// 			csAccessControl.Format("%s = %d \r\n", 
// 				ConvertString("support access count "),
// 				stuCap.nAccessControlGroups);
// 			csCap += csAccessControl;
// 			csCap += "\r\n";
// 		}
// 		else
// 		{
// 			return FALSE;
// 		}
// 	}
// 	else
// 	{
// 		return FALSE;
// 	}
// 	return TRUE;
}

BOOL DeviceImpl::StartGetFingerPrint()
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_CTRL_CAPTURE_FINGER_PRINT stuCapParam = {sizeof(stuCapParam)};
	stuCapParam.nChannelID = 0;
	strncpy(stuCapParam.szReaderID, "1", sizeof(stuCapParam.szReaderID)-1);
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_CAPTURE_FINGER_PRINT, &stuCapParam, 5000) == TRUE ? true : false;
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::SetDecodeInfo(const char *PKey)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	NET_IN_SET_QRCODE_DECODE_INFO stuInParam = {sizeof(stuInParam)};
	NET_OUT_SET_QRCODE_DECODE_INFO stuOutParam = {sizeof(stuOutParam)};
	
	stuInParam.emCipher = NET_ENUM_QRCODE_CIPHER_AES256;
	
	strncpy(stuInParam.szKey, PKey, 32);
	
	BOOL bRet = CLIENT_OperateAccessControlManager(m_lLoginID, NET_EM_ACCESS_CTL_SET_QRCODEDECODE_INFO, &stuInParam, &stuOutParam,5000) ;
	if (bRet)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DeviceImpl::EncryptString(const char *PKey,const char *pCardNo,char *pString)
{
	if (0 == m_lLoginID)
	{
		return FALSE;
	}
	
	NET_IN_ENCRYPT_STRING stuInParam = {sizeof(stuInParam)};
	NET_OUT_ENCRYPT_STRING stuOutParam = {sizeof(stuOutParam)};
	
	strncpy(stuInParam.szKey, PKey, 32);
	strncpy(stuInParam.szCard, pCardNo, 32);
	BOOL bRet = CLIENT_EncryptString(&stuInParam, &stuOutParam,5000);
	if (bRet)
	{
		strncpy(pString, stuOutParam.szEncryptString, 1023);
		return TRUE;
	}
	return FALSE;
}