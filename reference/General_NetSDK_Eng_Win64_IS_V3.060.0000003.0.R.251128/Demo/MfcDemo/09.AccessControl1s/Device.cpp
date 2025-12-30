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

//ÍøÂçÅäÖÃ
BOOL Device::GetNetWork(CString& csIp, CString& csMask, CString& csGateway)
{
	IMPL_IS_OK
	return m_pDevImpl->GetNetWork(csIp, csMask, csGateway);
}

BOOL Device::SetNetWork(CString csIp, CString csMask, CString csGateway)
{
	IMPL_IS_OK
	return m_pDevImpl->SetNetWork(csIp, csMask, csGateway);
}

BOOL Device::GetRegisterCFG(CFG_DVRIP_INFO& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetRegisterCFG(stuInfo);
}

BOOL Device::SetRegisterCFG(CFG_DVRIP_INFO* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetRegisterCFG(stuInfo);
}

BOOL Device::GetDevTime(NET_TIME& DeviceTime)
{
	IMPL_IS_OK
	return m_pDevImpl->GetDevTime(DeviceTime);
}

BOOL Device::SetDevTime(NET_TIME* pDeviceTime)
{
	IMPL_IS_OK
	return m_pDevImpl->SetDevTime(pDeviceTime);
}

BOOL Device::GetNTP(CFG_NTP_INFO& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetNTP(stuInfo);
}

BOOL Device::SetNTP(CFG_NTP_INFO* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetNTP(stuInfo);
}

BOOL Device::GetDST(AV_CFG_Locales& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetDST(stuInfo);
}

BOOL Device::SetDST(AV_CFG_Locales* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetDST(stuInfo);
}

BOOL Device::GetCapability(CString& csCap)
{
	IMPL_IS_OK
	return m_pDevImpl->GetCapability(csCap);
}

BOOL Device::GetVersion(CString& csVer)
{
	IMPL_IS_OK
	return m_pDevImpl->GetVersion(csVer);
}

BOOL Device::GetMAC(CString& csMAC)
{
	IMPL_IS_OK
	return m_pDevImpl->GetMAC(csMAC);
}

BOOL Device::GetMCUVer(CString& csMCUVer)
{
	IMPL_IS_OK
	return m_pDevImpl->GetMCUVer(csMCUVer);
}

BOOL Device::GetAccessCount(int& nCount)
{
	IMPL_IS_OK
	return m_pDevImpl->GetAccessCount(nCount);
}

BOOL Device::DevReboot()
{
	IMPL_IS_OK
	return m_pDevImpl->DevReboot();
}

BOOL Device::DevReset()
{
	IMPL_IS_OK
	return m_pDevImpl->DevReset();
}

BOOL Device::ModifyPassword(CString strName, CString strOldPassword, CString strNewPassword)
{
	IMPL_IS_OK
	return m_pDevImpl->ModifyPassword(strName, strOldPassword, strNewPassword);
}

BOOL Device::GetAutoMatrix(DHDEV_AUTOMT_CFG& stuAutoMT)
{
	IMPL_IS_OK
	return m_pDevImpl->GetAutoMatrix(stuAutoMT);
}

BOOL Device::SetAutoMatrix(DHDEV_AUTOMT_CFG* stuAutoMT)
{
	IMPL_IS_OK
	return m_pDevImpl->SetAutoMatrix(stuAutoMT);
}

BOOL Device::StartUpgrade(char *pchFileName, fUpgradeCallBack cbUpgrade, LDWORD dwUser)
{
	IMPL_IS_OK
	return m_pDevImpl->StartUpgrade(pchFileName, cbUpgrade, dwUser);
}

BOOL Device::StopUpgrade()
{
	IMPL_IS_OK
	return m_pDevImpl->StopUpgrade();
}

BOOL Device::GetTimeSchedule(int nChannelID, CFG_ACCESS_TIMESCHEDULE_INFO& stuTimeSchedule)
{
	IMPL_IS_OK
	return m_pDevImpl->GetTimeSchedule(nChannelID, stuTimeSchedule);
}

BOOL Device::SetTimeSchedule(int nChannelID, CFG_ACCESS_TIMESCHEDULE_INFO* stuTimeSchedule)
{
	IMPL_IS_OK
	return m_pDevImpl->SetTimeSchedule(nChannelID, stuTimeSchedule);
}

BOOL Device::GetAccessEventCfg(int nChannelID, CFG_ACCESS_EVENT_INFO& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetAccessEventCfg(nChannelID, stuInfo);
}

BOOL Device::SetAccessEventCfg(int nChannelID, CFG_ACCESS_EVENT_INFO* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetAccessEventCfg(nChannelID, stuInfo);
}

BOOL Device::InsertHolidayRecorde(NET_RECORDSET_HOLIDAY* stuInfo, int& nRecNo)
{
	IMPL_IS_OK
	return m_pDevImpl->InsertHolidayRecorde(stuInfo,nRecNo);
}
BOOL Device::GetHolidayRecorde(NET_RECORDSET_HOLIDAY& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetHolidayRecorde(stuInfo);
}
BOOL Device::UpdateHolidayRecorde(NET_RECORDSET_HOLIDAY* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->UpdateHolidayRecorde(stuInfo);
}
BOOL Device::RemoveHolidayRecorde(NET_RECORDSET_HOLIDAY* stuInfo)
{	
	IMPL_IS_OK
	return m_pDevImpl->RemoveHolidayRecorde(stuInfo);
}
BOOL Device::ClearHolidayRecorde()
{
	IMPL_IS_OK
	return m_pDevImpl->ClearHolidayRecorde();
}

BOOL Device::InsertPWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD* stuInfo, int& nRecNo)
{
	IMPL_IS_OK
	return m_pDevImpl->InsertPWDRecorde(stuInfo,nRecNo);
}
BOOL Device::GetPWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetPWDRecorde(stuInfo);
}
BOOL Device::UpdatePWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->UpdatePWDRecorde(stuInfo);
}
BOOL Device::RemovePWDRecorde(NET_RECORDSET_ACCESS_CTL_PWD* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->RemovePWDRecorde(stuInfo);
}
BOOL Device::ClearPWDRecorde()
{
	IMPL_IS_OK
	return m_pDevImpl->ClearPWDRecorde();
}

BOOL Device::InsertCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD* stuInfo, BOOL bEx, int& nRetNo)
{
	IMPL_IS_OK
	return m_pDevImpl->InsertCardRecorde(stuInfo, bEx, nRetNo);
}

BOOL Device::GetCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetCardRecorde(stuInfo);
}

BOOL Device::UpdateCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD* stuInfo, BOOL bEx, int& nRetNo)
{
	IMPL_IS_OK
	return m_pDevImpl->UpdateCardRecorde(stuInfo, bEx, nRetNo);
}

BOOL Device::RemoveCardRecorde(NET_RECORDSET_ACCESS_CTL_CARD* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->RemoveCardRecorde(stuInfo);
}

BOOL Device::ClearCardRecorde()
{
	IMPL_IS_OK
	return m_pDevImpl->ClearCardRecorde();
}

BOOL Device::GetOpenDoorGroup(int nChannelID, CFG_OPEN_DOOR_GROUP_INFO& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetOpenDoorGroup(nChannelID, stuInfo);
}

BOOL Device::SetOpenDoorGroup(int nChannelID, CFG_OPEN_DOOR_GROUP_INFO* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetOpenDoorGroup(nChannelID, stuInfo);
}

BOOL Device::GetAccessGeneralInfo(CFG_ACCESS_GENERAL_INFO& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetAccessGeneralInfo(stuInfo);
}
BOOL Device::SetAccessGeneralInfo(CFG_ACCESS_GENERAL_INFO* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetAccessGeneralInfo(stuInfo);
}

BOOL Device::GetOpenDoorRouteInfo(CFG_OPEN_DOOR_ROUTE_INFO& stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->GetOpenDoorRouteInfo(stuInfo);
}

BOOL Device::SetOpenDoorRouteInfo(CFG_OPEN_DOOR_ROUTE_INFO* stuInfo)
{
	IMPL_IS_OK
	return m_pDevImpl->SetOpenDoorRouteInfo(stuInfo);
}

BOOL Device::CardRecStartFind()
{
	IMPL_IS_OK
	return m_pDevImpl->CardRecStartFind();
}

BOOL Device::CardRecFindNext(int nMaxNum, NET_RECORDSET_ACCESS_CTL_CARDREC* pstuCardRec, int& nRecordNum)
{
	IMPL_IS_OK
	return m_pDevImpl->CardRecFindNext(nMaxNum, pstuCardRec, nRecordNum);
}

BOOL Device::CardRecStopFind()
{
	IMPL_IS_OK
	return m_pDevImpl->CardRecStopFind();
}

BOOL Device::CardRecQueryCount(int& nCount)
{
	IMPL_IS_OK
	return m_pDevImpl->CardRecQueryCount(nCount);
}

BOOL Device::AlarmStartFind()
{
	IMPL_IS_OK
	return m_pDevImpl->AlarmStartFind();
}
BOOL Device::AlarmFindNext(int nMaxNum, NET_RECORD_ACCESS_ALARMRECORD_INFO* pstuAlarm, int& nRecordNum)
{
	IMPL_IS_OK
	return m_pDevImpl->AlarmFindNext(nMaxNum, pstuAlarm, nRecordNum);
}
BOOL Device::AlarmStopFind()
{
	IMPL_IS_OK
	return m_pDevImpl->AlarmStopFind();
}
BOOL Device::AlarmQueryCount(int& nCount)
{
	IMPL_IS_OK
	return m_pDevImpl->AlarmQueryCount(nCount);
}

BOOL Device::LogStartFind()
{
	IMPL_IS_OK
	return m_pDevImpl->LogStartFind();
}
BOOL Device::LogFindNext(int nMaxNum, NET_LOG_INFO* pstuAlarm, int& nRecordNum)
{
	IMPL_IS_OK
	return m_pDevImpl->LogFindNext(nMaxNum, pstuAlarm, nRecordNum);
}
BOOL Device::LogStopFind()
{
	IMPL_IS_OK
	return m_pDevImpl->LogStopFind();
}
BOOL Device::LogQueryCount(int& nCount)
{
	IMPL_IS_OK
	return m_pDevImpl->LogQueryCount(nCount);
}

BOOL Device::OpenDoor(int nChanne)
{
	IMPL_IS_OK
	return m_pDevImpl->OpenDoor(nChanne);
}

BOOL Device::CloseDoor(int nChanne)
{
	IMPL_IS_OK
	return m_pDevImpl->CloseDoor(nChanne);
}

BOOL Device::OpenAlways(int nChanne)
{
	IMPL_IS_OK
	return m_pDevImpl->OpenAlways(nChanne);
}

BOOL Device::CloseAlways(int nChanne)
{
	IMPL_IS_OK
	return m_pDevImpl->CloseAlways(nChanne);
}

BOOL Device::GetDoorState(int nChanne, EM_NET_DOOR_STATUS_TYPE& emStateType)
{
	IMPL_IS_OK
	return m_pDevImpl->GetDoorState(nChanne, emStateType);
}

BOOL Device::GetLoginState()
{
	IMPL_IS_OK
	return m_pDevImpl->GetLoginState();
}

EM_DEVICE_ERR Device::GetDeviceErr()
{
	if (m_pDevImpl == NULL)
	{
		return EM_UNKNOWN_ERR;
	}
	return m_pDevImpl->GetDeviceErr();
}

DWORD Device::GetLastError()
{
	return m_pDevImpl->GetLastError();
}


BOOL Device::StartGetFingerPrint()
{
	IMPL_IS_OK
	return m_pDevImpl->StartGetFingerPrint();
}