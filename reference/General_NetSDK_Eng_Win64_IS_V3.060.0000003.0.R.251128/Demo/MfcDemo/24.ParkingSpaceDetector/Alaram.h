#ifndef _ALARM_H_
#define _ALARM_H_

struct AlarmData{
	LLONG	lAnalyzerHadnle;
	DWORD	dwAlarmType;
	void*	pAlarmInfo;
	BYTE*	pPictureBuffer;
	DWORD	dwPictureBufferSize;
	int		nSequence;

	AlarmData() : lAnalyzerHadnle(0), dwAlarmType(0), pAlarmInfo(NULL), pPictureBuffer(NULL), dwPictureBufferSize(0), nSequence(0)
	{		
	}
};

struct StuEventInfo;
class CAlaramImpl;

/************************************************************************/
/* Alarm info class*/
/************************************************************************/
class CAlaram
{
public:
	CAlaram(void);
	~CAlaram(void);

	bool			SetAlarmData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE* pBuffer, DWORD dwBufSize, int nSequence);

	BYTE*			GetPictureInfo(DWORD& dwPictureBufferSize);

	void			GetEventInfo(StuEventInfo& stuEventInfo);

	void			DestroyParam();

	std::string		GetGuid() const;

private:
	CAlaramImpl*	m_pAlarmImpl;
};

#endif