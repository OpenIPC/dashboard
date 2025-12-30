#if !defined(_LOCK_)
#define _LOCK_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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