#include "stdafx.h"
#include "Lock.h"

Mutex::Mutex()
{
	InitializeCriticalSection(&m_critclSection);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&m_critclSection);
}

int Mutex::Lock()
{
	EnterCriticalSection(&m_critclSection);
	return 0;
}

int Mutex::UnLock()
{
	LeaveCriticalSection(&m_critclSection);
	return 0;
}



CLock::CLock(Mutex& mutex) : m_mutex(mutex), m_bLock(true)
{
	m_mutex.Lock();
}

CLock::~CLock()
{
	UnLock();
}

void CLock::UnLock()
{
	if (m_bLock)
	{
		m_mutex.UnLock();
		m_bLock = false;
	}
}