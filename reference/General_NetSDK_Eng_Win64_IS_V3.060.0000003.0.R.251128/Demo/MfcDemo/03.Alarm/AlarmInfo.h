// AlarmInfo.h: interface for the CAlarmInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALARMINFO_H__720FA033_A07C_4017_9730_C5CDD93D14D5__INCLUDED_)
#define AFX_ALARMINFO_H__720FA033_A07C_4017_9730_C5CDD93D14D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum ALARM_STATUS
{
	ALARM_START,
	ALARM_STOP,
};

class CAlarmInfoEx
{
public:
	explicit CAlarmInfoEx();
	~CAlarmInfoEx();
public:	
	int				m_nAlarmType;		// Alarm type 	
	int				m_nChannel;			// Channel
	ALARM_STATUS	m_emAlarmStatus;	// Alarm Staus
	CString			m_strTime;			// Time

};
#endif // !defined(AFX_ALARMINFO_H__720FA033_A07C_4017_9730_C5CDD93D14D5__INCLUDED_)





















