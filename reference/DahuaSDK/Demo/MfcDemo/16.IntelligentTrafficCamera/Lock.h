#ifndef _LOCK_
#define _LOCK_
#pragma once
#include <Windows.h>
class CLock
{
public:
	explicit CLock(CRITICAL_SECTION& criticalSection);
	~CLock();

private:
	CRITICAL_SECTION& m_criticalSection;
};

#endif