// DevMgr.h: interface for the CDevMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DEVMGR_H_
#define _DEVMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetSDKDemo.h"
#pragma warning(disable:4786)
#include <list>

using namespace std;

typedef struct 
{
	LONG alarmType;
	DEV_STATE state;
	CTime	timeStamp;
}AlarmNode;

class CDevMgr  
{
public:
	class SearchDevByHandle;
	friend class SearchDevByHandle;

public:
	CDevMgr();
	~CDevMgr();

	static CDevMgr& GetDevMgr(void);

	//protected copy object
private:
	CDevMgr(const CDevMgr&);
	CDevMgr& operator=(const CDevMgr&);

public:

	//加入节点
	int  PushBack(DeviceNode* node);
	
	//删除节点
	int  DelNode(LLONG lLoginID);

	//修改节点
	int	 ModifyNode(LLONG lLoginID, BOOL bIsOnline);

	//回调函数返回值说明：1 结束枚举， 0 继续枚举
	void For_EachDev(int (* cbForEach)(const DeviceNode& node, LDWORD dwUser), LDWORD dwUser);

	//回调函数返回值说明：1 结束枚举， 0 继续枚举
	void For_EachAlmNode(int (* cbForEachAlm)(const AlarmNode& node, LDWORD dwUser), LDWORD dwUser);
	
	//返回的数据不能做长久保存，下次要重新获取
	int  GetDev(LLONG lLoginID, DeviceNode& node);

	//写入回调出来的报警数据
	int SetAlarmInfo(LLONG lLoginID, LONG lCommand, char *pchDVRIP, LONG nDVRPort, char *pBuf, DWORD dwBufLen);
	int CDevMgr::GetAlarmInfo(LLONG lLoginID, DEV_STATE *des);

	BOOL IsOnline(LLONG lLoginID);

	BOOL IsOnline(DeviceNode* node);

private:

	list<DeviceNode*> m_lstDevice;
	list<AlarmNode*> m_lstAlarm;
	CRITICAL_SECTION  m_csDev;
	CRITICAL_SECTION  m_csAlarm;
	CRITICAL_SECTION  m_csAlmList;
};

class CDevMgr::SearchDevByHandle
{
	LLONG m_lLoginID;
public:
	SearchDevByHandle(const LLONG& lLoginID):m_lLoginID(lLoginID){}

	bool operator()( DeviceNode* const node)
	{
		if (!node)
		{
			return false;
		}

		return (m_lLoginID==node->LoginID)?true:false;
	}
};

#endif // _DEVMGR_H_
