#include "stdafx.h"
#include "Alarm.h"

AlarmData::AlarmData()
{
	InitEventParam();
	InitStuAlarmItem();
}

AlarmData::~AlarmData()
{
	ReleaseEventParam();
	ReleaseStuAlarmItem();
}

void AlarmData::InitEventParam()
{
	m_stueventParam = NEW EventParam;
}

void AlarmData::ReleaseEventParam()
{
	if (NULL != m_stueventParam)
	{
		delete[] m_stueventParam->pBuffer;
		delete[] m_stueventParam->pEventInfo;
		delete m_stueventParam;
		m_stueventParam = NULL;
	}
}

void AlarmData::InitStuAlarmItem()
{
	m_stualarmItem = NEW StuAlarmItem;
}

void AlarmData::ReleaseStuAlarmItem()
{
	if (NULL != m_stualarmItem)
	{
		delete[] m_stualarmItem->pAlarmInfo;
		delete[] m_stualarmItem->pBuffer;
		delete[] m_stualarmItem->szGUID;
		delete m_stualarmItem;
		m_stualarmItem = NULL;
	}
}

EventParam* AlarmData::GetEventParam()
{
	return m_stueventParam;
}
StuAlarmItem* AlarmData::GetAlarmItem()
{
	return m_stualarmItem;
}
std::string AlarmData::GetstrGUID()
{
	return m_strGUID;
}

StuEventInfo AlarmData::GetEventInfo()
{
	return m_stuEventInfo;
}

void AlarmData::SetEventInfo(StuEventInfo &stuEventInfo)
{
	m_stuEventInfo = stuEventInfo;
}

BOOL AlarmData::SetData(LLONG lAnalyzerHandle,  void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize,  int nSequence)
{
	m_stueventParam->lAnalyzerHandle = lAnalyzerHandle;
	m_stueventParam->dwEventType = EVENT_IVS_TRAFFICJUNCTION;
	m_stueventParam->dwBufSize = dwBufSize;
	m_stueventParam->dwOffset = m_stuEventInfo.dwOffset;
	m_stueventParam->dwOffsetLength = m_stuEventInfo.dwOffsetLength;
	m_stueventParam->nSequence = nSequence;
	m_stueventParam->strDeviceAddress = m_stuEventInfo.strDeviceAddress; // note, szDeviceAddress in pAlarmInfo is allocated by NetSDK, so needs to save szDeviceAddress first
	m_stueventParam->strUTCTime = m_stuEventInfo.strUTCTime;

	if (dwBufSize > 0)
	{
		m_stueventParam->pBuffer = NEW BYTE[dwBufSize]; // if dwBufSize is zero, then eventData->pBuffer is NULL
		if (NULL == m_stueventParam->pBuffer)
		{
			ReleaseEventParam();
			return FALSE;
		}
		memset(m_stueventParam->pBuffer, 0, dwBufSize);	
		memcpy(m_stueventParam->pBuffer, pBuffer, dwBufSize);		
	}	

	m_stueventParam->pEventInfo = NEW char[m_stuEventInfo.nStructSize];
	if (NULL == m_stueventParam->pEventInfo /*&& dwBufSize > 0*/)
	{
		ReleaseEventParam();
		return FALSE;
	}

	memset(m_stueventParam->pEventInfo, 0, m_stuEventInfo.nStructSize);
	memcpy(m_stueventParam->pEventInfo, pAlarmInfo, m_stuEventInfo.nStructSize);

	GUID guid = {0};
	HRESULT hr =  CoCreateGuid(&guid);
	std::string strGUID(m_stuEventInfo.strUTCTime);
	strGUID.append(GuidToString(guid));	
	m_strGUID = strGUID;

	/////////////////
	int nStructSize = 0;
	nStructSize = m_stuEventInfo.nStructSize;
	char* pStructInfo = NEW char[nStructSize];
	if (NULL == pStructInfo)
	{
		return FALSE;
	}
	memset(pStructInfo, 0, nStructSize);	
	memcpy(pStructInfo, m_stueventParam->pEventInfo, nStructSize);		

	BYTE* pBuf = NULL;
	if (m_stueventParam->pBuffer != NULL)
	{
		if (m_stueventParam->dwOffsetLength <= 0)
		{
			delete[] pStructInfo;
			return FALSE;
		}
		pBuf = NEW BYTE[m_stueventParam->dwOffsetLength];
		if (NULL == pBuf)
		{
			delete[] pStructInfo;
			return FALSE;
		}
		memset(pBuf, 0, m_stueventParam->dwOffsetLength);
		memcpy(pBuf, m_stueventParam->pBuffer + m_stueventParam->dwOffset, m_stueventParam->dwOffsetLength);
	}	

	char* szGUIDTmp = NEW char[64];
	if (NULL == szGUIDTmp)
	{
		delete[] pStructInfo;
		if (pBuf != NULL)
		{
			delete[] pBuf;			
		}
		return FALSE;		
	}
	memset(szGUIDTmp, 0, 64);
	strncpy(szGUIDTmp, strGUID.c_str(), 63);	

	m_stualarmItem->dwAlarmType = m_stueventParam->dwEventType;
	m_stualarmItem->pAlarmInfo = pStructInfo;	
	m_stualarmItem->nLen       = nStructSize;
	m_stualarmItem->nCheckFlag = 0;//Not checked
	m_stualarmItem->strDeviceAddress = m_stueventParam->strDeviceAddress;	

	m_stualarmItem->pBuffer		= pBuf;
	if (m_stueventParam->dwBufSize > 0)
	{
		m_stualarmItem->dwBufSize	= m_stueventParam->dwOffsetLength;
	}	
	m_stualarmItem->szGUID		= szGUIDTmp;
	return TRUE;
}

Alarm::Alarm()
{
	pAlarmData = new AlarmData;
}

Alarm::~Alarm()
{
	if (NULL != pAlarmData)
	{
		delete pAlarmData;
		pAlarmData = NULL;
	}
}

void Alarm::DestroyParam()
{
	if (NULL != pAlarmData)
	{
		pAlarmData->ReleaseEventParam();
	}
}

BOOL Alarm::CreateParam(LLONG lAnalyzerHandle,  void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize,  int nSequence)
{
	BOOL bRet = GetStructInfo(pAlarmInfo);
	if (FALSE == bRet)
	{		
		return FALSE;
	}	
	return pAlarmData->SetData(lAnalyzerHandle, pAlarmInfo, pBuffer, dwBufSize, nSequence);
}

std::string Alarm::GetGUID()
{
	return pAlarmData->GetstrGUID();
}


TrafficJunctionAlarm::TrafficJunctionAlarm()
{
	
}

TrafficJunctionAlarm::~TrafficJunctionAlarm()
{

}

BOOL TrafficJunctionAlarm::GetStructInfo(void* pEventInfo)
{
	if (NULL == pEventInfo)
	{
		return FALSE;
	}
	StuEventInfo stuEventInfo;
	stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFICJUNCTION_INFO);
	DEV_EVENT_TRAFFICJUNCTION_INFO* pInfo = (DEV_EVENT_TRAFFICJUNCTION_INFO*)pEventInfo;
	stuEventInfo.strCountNum = "0";// set "0" as default
	stuEventInfo.strTime = NetTimeEx2Str(pInfo->UTC);
	stuEventInfo.strEventType = EventType2Str(EVENT_IVS_TRAFFICJUNCTION);
	stuEventInfo.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
	stuEventInfo.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
	stuEventInfo.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				
	stuEventInfo.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
	stuEventInfo.strPlateColor = pInfo->stTrafficCar.szPlateColor;
	stuEventInfo.strPlateType = pInfo->stTrafficCar.szPlateType;
	stuEventInfo.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
	stuEventInfo.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;
	stuEventInfo.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
	stuEventInfo.strLane  = Int2Str(pInfo->stTrafficCar.nLane);
	stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
	stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
	stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
	stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
	stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
	this->pAlarmData->SetEventInfo(stuEventInfo);
	return TRUE;
}

ManualSnapAlarm::ManualSnapAlarm()
{
}

ManualSnapAlarm::~ManualSnapAlarm()
{
}

BOOL ManualSnapAlarm::GetStructInfo(void* pEventInfo)
{
	if (NULL == pEventInfo)
	{
		return FALSE;
	}
	StuEventInfo stuEventInfo;
	stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_MANUALSNAP_INFO);
	DEV_EVENT_TRAFFIC_MANUALSNAP_INFO* pInfo = (DEV_EVENT_TRAFFIC_MANUALSNAP_INFO*)pEventInfo;
	stuEventInfo.strCountNum = "0";
	stuEventInfo.strTime = NetTimeEx2Str(pInfo->UTC);
	stuEventInfo.strEventType = EventType2Str(EVENT_IVS_TRAFFIC_MANUALSNAP);
	stuEventInfo.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
	stuEventInfo.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
	stuEventInfo.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				
	stuEventInfo.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
	stuEventInfo.strPlateColor = pInfo->stTrafficCar.szPlateColor;
	stuEventInfo.strPlateType = pInfo->stTrafficCar.szPlateType;
	stuEventInfo.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
	stuEventInfo.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;
	stuEventInfo.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
	stuEventInfo.strLane  = Int2Str(pInfo->stTrafficCar.nLane);
	stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
	stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
	stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
	stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
	stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
	this->pAlarmData->SetEventInfo(stuEventInfo);
	return TRUE;
}