#pragma once


class CFaceMutex
{
public:
	CFaceMutex();
	~CFaceMutex();

	int	Lock();
	int	UnLock();

private:
	CRITICAL_SECTION m_critclSection;
};

class FaceLock
{
public:
	FaceLock(CFaceMutex& mutex) : m_mutex(mutex), m_bLock(true)
	{
		m_mutex.Lock();
	}

	~FaceLock()
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
	CFaceMutex& m_mutex;
	bool m_bLock;
};