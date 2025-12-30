#if !defined(_ALARM_)
#define _ALARM_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Utility.h"

struct StuEventInfoDisplay
{
	std::string strCountNum;			// count num
	std::string strTime;				// time
	std::string strEventType;			// event type

	std::string strFileIndex;			// file index
	std::string strFileCount;			// file count
	std::string strGroupId;				// group id

	std::string strPlateNumber;			// plate text 
	std::string strPlateColor;			// plate color
	std::string strPlateType;			// plate type

	std::string	strVehicleType;			// vehicle type
	std::string strVehicleColor;		// vehicle color
	std::string strVehicleSize;			// vehicle size

	std::string strLane;				// lane number	
	std::string strDeviceAddress;		// device address
};

struct StuEventInfo : public StuEventInfoDisplay
{
	int					nStructSize;			// event struct size
	std::string			strDeviceAddress;		// event address
	DH_RECT				rectBoundingBox;		
	DWORD				dwOffset;
	DWORD				dwOffsetLength;
	std::string			strUTCTime;

	StuEventInfo() : nStructSize(0), strDeviceAddress(""), dwOffset(0), dwOffsetLength(0), strUTCTime("")
	{		
		rectBoundingBox.bottom = 0;
		rectBoundingBox.left = 0;
		rectBoundingBox.right = 0;
		rectBoundingBox.top = 0;
	}
};

struct EventParam
{
	LLONG	lAnalyzerHandle;
	DWORD	dwEventType;		// event type
	void*	pEventInfo;			// event info
	BYTE*	pBuffer;			// picture info
	DWORD	dwBufSize;			// pBuffer size
	DWORD	dwOffset;			// plate picture offset in pBuffer
	DWORD	dwOffsetLength;		// plate picture length
	int		nSequence;
	std::string	strDeviceAddress;	// event address
	std::string	strUTCTime;			// UTC time, format is %04d%02d%02d%02d%02d%02d%03d

	EventParam() : lAnalyzerHandle(0), dwEventType(0), pEventInfo(0), pBuffer(0), dwBufSize(0), dwOffset(0), dwOffsetLength(0), nSequence(0), strDeviceAddress(""), strUTCTime("")
	{

	}
	~EventParam()
	{

	}
};

struct StuListItemData
{
	char*	szGUID;		// GUID
	BYTE*	pBuffer;	// License Plate image buffer
	DWORD	dwBufSize;	// License Plate image size

	StuListItemData() : szGUID(NULL), pBuffer(NULL), dwBufSize(0)
	{

	}
	~StuListItemData()
	{

	}
};

struct StuAlarmItem : public StuListItemData
{	
	DWORD			dwAlarmType;
	int				nLen;
	void*			pAlarmInfo;
	int				nCheckFlag;
	std::string		strDeviceAddress;

	StuAlarmItem(): dwAlarmType(0), nLen(0), pAlarmInfo(NULL), nCheckFlag(0), strDeviceAddress("")
	{

	}	
	~StuAlarmItem()
	{

	}
};


class AlarmData
{
public:
	AlarmData();
	~AlarmData();
	void InitEventParam();
	void ReleaseEventParam();

	void InitStuAlarmItem();
	void ReleaseStuAlarmItem();

	EventParam* GetEventParam();

	StuAlarmItem* GetAlarmItem();

	std::string GetstrGUID();

	StuEventInfo GetEventInfo();
	void SetEventInfo(StuEventInfo &stuEventInfo);

	BOOL SetData(LLONG lAnalyzerHandle,  void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize,  int nSequence);
private:
	EventParam* m_stueventParam;
	StuAlarmItem* m_stualarmItem;
	std::string m_strGUID;
	StuEventInfo m_stuEventInfo;
};

class Alarm
{
public:
	virtual ~Alarm();
	BOOL CreateParam(LLONG lAnalyzerHandle,  void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize,  int nSequence);
	void DestroyParam();
	std::string GetGUID();
public:
	AlarmData* pAlarmData;
protected:
	virtual BOOL GetStructInfo(void* pEventInfo) = 0;
	Alarm();	
};

class TrafficJunctionAlarm : public Alarm
{
public:
	TrafficJunctionAlarm();
	~TrafficJunctionAlarm();
	BOOL GetStructInfo(void* pEventInfo);
};

class ManualSnapAlarm : public Alarm
{
public:
	ManualSnapAlarm();
	~ManualSnapAlarm();
	BOOL GetStructInfo(void* pEventInfo);
};
#endif