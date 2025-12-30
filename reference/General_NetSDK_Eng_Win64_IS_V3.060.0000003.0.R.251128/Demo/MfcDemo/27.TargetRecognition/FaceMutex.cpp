#include "stdafx.h"
#include "FaceMutex.h"


CFaceMutex::CFaceMutex()
{
	InitializeCriticalSection(&m_critclSection);
}

CFaceMutex::~CFaceMutex()
{
	DeleteCriticalSection(&m_critclSection);
}

int CFaceMutex::Lock()
{
	EnterCriticalSection(&m_critclSection);
	return 0;
}

int CFaceMutex::UnLock()
{
	LeaveCriticalSection(&m_critclSection);
	return 0;
}