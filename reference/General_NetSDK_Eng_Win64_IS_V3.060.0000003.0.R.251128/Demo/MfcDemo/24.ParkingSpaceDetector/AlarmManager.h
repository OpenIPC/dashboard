#ifndef _ALARM_MANAGER_H_
#define _ALARM_MANAGER_H_

class CAlaram;
class CAlarmManagerImpl;

/************************************************************************/
/* Alarm Manager Class              */
/************************************************************************/
class CAlarmManager
{
public:
	CAlarmManager(void);
	~CAlarmManager(void);

	//retun handle of alarm
	LLONG		AddAlarm(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE* pBuffer, DWORD dwBufSize, int nSequence);

	void		ClearAllAlarm();

	CAlaram*	GetAlarm(LLONG lAlarmHandle);

private:
	CAlarmManagerImpl*	m_pAlarmManagerImpl;
};
#endif