#ifndef _LOCK_
#define _LOCK_

#include <Windows.h>
class Mutex
{
public:
	Mutex();
	~Mutex();

	int	Lock();
	int	UnLock();

private:
	CRITICAL_SECTION m_critclSection;
};

class  CLock
{
public:
	explicit CLock(Mutex& mutex);

	~CLock();

	void UnLock();

private:
	Mutex& m_mutex;
	bool m_bLock;
};

#endif