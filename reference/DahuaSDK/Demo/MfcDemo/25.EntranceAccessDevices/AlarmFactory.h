#if !defined(_ALARM_FACTORY_)
#define _ALARM_FACTORY_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Alarm.h"
#include "Lock.h"
#include <deque>

class AlarmFactory
{
public:
	static AlarmFactory &GetInstance();
	Alarm* CreateAlarm(DWORD dwAlarmType);
	void AddAlarm(Alarm* alarm);
	void ClearAlarm();
	Alarm* GetAlarm(std::string str);
private:
	AlarmFactory();
	~AlarmFactory();
	AlarmFactory(const AlarmFactory& alarmfactory);
	AlarmFactory& operator=(const AlarmFactory& alarmfactory);
	static AlarmFactory*	mInstance;
private:
	std::deque<Alarm*>			m_deqAlarm;
	CRITICAL_SECTION		m_csMap;
};
#endif