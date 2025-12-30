#ifndef _ATTENDANCE_MANAGERIMPL_H_
#define _ATTENDANCE_MANAGERIMPL_H_
#include  "../../../Include/Common/dhnetsdk.h"

class CAttendanceManagerImpl
{
public:
	CAttendanceManagerImpl();
	~CAttendanceManagerImpl();
	friend class CAttendanceManager;
private:
	LLONG m_lDeviceHandle;
	LLONG m_lRealLoadHandle;
};
#endif