#include "stdafx.h"
#include "AlarmFactory.h"

#define MAX_EVENT_NUM 100

AlarmFactory *AlarmFactory::mInstance = NULL;
AlarmFactory &AlarmFactory::GetInstance()
{
	if (mInstance == NULL)
	{
		mInstance = NEW AlarmFactory;
	}
	return *mInstance;
}

AlarmFactory::AlarmFactory()
{
	InitializeCriticalSection(&m_csMap);
}

AlarmFactory::~AlarmFactory()
{
	DeleteCriticalSection(&m_csMap);
}

Alarm* AlarmFactory::CreateAlarm( DWORD dwAlarmType)
{	
	Alarm* alarm = NULL;
	switch(dwAlarmType)
	{
	case EVENT_IVS_TRAFFICJUNCTION:
		alarm = NEW TrafficJunctionAlarm();
		break;
	case EVENT_IVS_TRAFFIC_MANUALSNAP:
		alarm = NEW ManualSnapAlarm();
		break;
	default:
		break;
	}
	return alarm;
}

void AlarmFactory::AddAlarm(Alarm* alarm)
{
	CLock lock(m_csMap);
	m_deqAlarm.push_front(alarm);
	if (m_deqAlarm.size() > MAX_EVENT_NUM)
	{
		Alarm* alarm = m_deqAlarm[MAX_EVENT_NUM];
		delete alarm;
		alarm = NULL;
		m_deqAlarm.pop_back();
	}
}

void AlarmFactory::ClearAlarm()
{
	CLock lock(m_csMap);
	std::deque<Alarm*>::iterator ite = m_deqAlarm.begin();
	for (;ite!=m_deqAlarm.end();)
	{
		delete *ite;
		ite++;
	}
	m_deqAlarm.clear();
}

Alarm* AlarmFactory::GetAlarm(std::string str)
{
	CLock lock(m_csMap);
	std::deque<Alarm*>::iterator ite = m_deqAlarm.begin();
	for (;ite!=m_deqAlarm.end();)
	{
		if (str == (*ite)->pAlarmData->GetstrGUID())
		{
			return *ite;
		}
		ite++;
	}
	return NULL;
}