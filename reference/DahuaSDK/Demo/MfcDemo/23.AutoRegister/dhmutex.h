#ifndef DHMUTEX_H
#define DHMUTEX_H

#if defined(WIN32) || defined(WIN64)
#include <windows.h>

/*
class CCriticalSection  
{
public:
    CCriticalSection() 
	{ 
		InitializeCriticalSection(&m_sec);
	} 
    ~CCriticalSection() { DeleteCriticalSection(&m_sec); }
public:
    void Lock() { EnterCriticalSection(&m_sec); }
    void Unlock() { LeaveCriticalSection(&m_sec); }
protected:
    CRITICAL_SECTION m_sec;
};
*/

class CCSLock
{
public:
	CCSLock(CRITICAL_SECTION& cs):m_cs(cs)
	{
		EnterCriticalSection(&m_cs);
	}

	~CCSLock()
	{
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION& m_cs;
};

#else

#include <pthread.h>

#endif

class DHMutex
{
public:
	DHMutex();
	~DHMutex();

	int	Lock();
	int	UnLock();

private:

#if defined(WIN32) || defined(WIN64)
	CRITICAL_SECTION m_critclSection;
#else
	pthread_mutex_t m_mutex;
#endif
};

class DHLock
{
public:
	DHLock(DHMutex& mutex) : m_mutex(mutex), m_bLock(true)
	{
		m_mutex.Lock();
	}
	
	~DHLock()
	{
		UnLock();
	}

	void UnLock()
	{
		if (m_bLock)
		{
			m_mutex.UnLock();
			m_bLock = false;
		}
	}
	
private:
	DHMutex& m_mutex;
	bool m_bLock;
};

#endif



