#include "stdafx.h"
#include "dhmutex.h"

DHMutex::DHMutex()
{
#if !(defined(WIN32) || defined(WIN64))
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	//attr.__mutexkind = PTHREAD_MUTEX_RECURSIVE_NP;
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex, &attr);
	//printf("pthread_mutex_init=%d!\n", ret);
	pthread_mutexattr_destroy(&attr);
#else
	InitializeCriticalSection(&m_critclSection);
#endif
}

DHMutex::~DHMutex()
{
#if !(defined(WIN32) || defined(WIN64))
	pthread_mutex_destroy(&m_mutex);
#else
	DeleteCriticalSection(&m_critclSection);
#endif
}

int DHMutex::Lock()
{
#if defined(WIN32) || defined(WIN64)
	EnterCriticalSection(&m_critclSection);
	return 0;
#else
	return pthread_mutex_lock(&m_mutex);
#endif
}

int DHMutex::UnLock()
{
#if defined(WIN32) || defined(WIN64)
	LeaveCriticalSection(&m_critclSection);
	return 0;
#else
	return pthread_mutex_unlock(&m_mutex);
#endif
}