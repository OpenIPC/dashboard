#include "stdafx.h"
#include "Lock.h"


CLock::CLock(CRITICAL_SECTION& criticalSection) : m_criticalSection(criticalSection)
{
	EnterCriticalSection(&m_criticalSection);
}


CLock::~CLock()
{
	LeaveCriticalSection(&m_criticalSection);
}
