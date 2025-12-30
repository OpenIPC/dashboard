#include "stdafx.h"
#include "AlarmManager.h"
#include "Alaram.h"
#include "Utils/Lock.h"
#include <deque>

#include <algorithm>

#define MAX_EVENT_NUM 100

class CAlarmManagerImpl
{
public:
	CAlarmManagerImpl(void);
	~CAlarmManagerImpl(void);

	LLONG		AddAlarm(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE* pBuffer, DWORD dwBufSize, int nSequence);

	void		ClearAllAlarm();

	CAlaram*	GetAlarm(LLONG lAlarmHandle);

private:
	Mutex				m_Mutex;
	std::deque<LLONG>	m_deqAlarm; // list of Alarm
};


CAlarmManagerImpl::CAlarmManagerImpl(void)
{
}

CAlarmManagerImpl::~CAlarmManagerImpl(void)
{
	ClearAllAlarm();
}


LLONG CAlarmManagerImpl::AddAlarm(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE* pBuffer, DWORD dwBufSize, int nSequence)
{
	CAlaram *pAlarm = NEW CAlaram;
	if (NULL == pAlarm)
	{
		return NULL;
	}

	pAlarm->SetAlarmData(lAnalyzerHandle, dwAlarmType, pAlarmInfo, pBuffer, dwBufSize, nSequence);

	LLONG lAlarmHandle = (LLONG)pAlarm;
	CLock lck(m_Mutex);
	m_deqAlarm.push_front(lAlarmHandle);
	if (m_deqAlarm.size() > MAX_EVENT_NUM)
	{
		CAlaram* pAlaram = (CAlaram*)m_deqAlarm[MAX_EVENT_NUM];
		delete pAlaram;
		m_deqAlarm.pop_back();
	}
	lck.UnLock();

	return lAlarmHandle;
}

void CAlarmManagerImpl::ClearAllAlarm()
{
	CLock lock(m_Mutex);
	std::deque<LLONG>::iterator ite = m_deqAlarm.begin();
	for (;ite!=m_deqAlarm.end();)
	{
		CAlaram *pTemp = (CAlaram*)(*ite);

		delete pTemp;
		ite++;
	}
	m_deqAlarm.clear();
}

CAlaram* CAlarmManagerImpl::GetAlarm(LLONG lAlarmHandle)
{
	CLock lck(m_Mutex);
	std::deque<LLONG>::iterator it = find(m_deqAlarm.begin(), m_deqAlarm.end(), lAlarmHandle);

	if (it == m_deqAlarm.end())
	{
		return NULL;
	}

	CAlaram* pTemp = (CAlaram*)(*it);
	return pTemp;
}


CAlarmManager::CAlarmManager(void)
{
	m_pAlarmManagerImpl = NEW CAlarmManagerImpl;
}

CAlarmManager::~CAlarmManager(void)
{
	if (m_pAlarmManagerImpl)
	{
		delete m_pAlarmManagerImpl;
		m_pAlarmManagerImpl = NULL;
	}
}


LLONG CAlarmManager::AddAlarm(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE* pBuffer, DWORD dwBufSize, int nSequence)
{
	if (m_pAlarmManagerImpl)
	{
		return m_pAlarmManagerImpl->AddAlarm(lAnalyzerHandle, dwAlarmType, pAlarmInfo, pBuffer, dwBufSize, nSequence);
	}
	return NULL;
}

void CAlarmManager::ClearAllAlarm()
{
	if (m_pAlarmManagerImpl)
	{
		m_pAlarmManagerImpl->ClearAllAlarm();
	}
}

CAlaram* CAlarmManager::GetAlarm(LLONG lAlarmHandle)
{
	if (m_pAlarmManagerImpl)
	{
		return m_pAlarmManagerImpl->GetAlarm(lAlarmHandle);
	}
	return NULL;
}