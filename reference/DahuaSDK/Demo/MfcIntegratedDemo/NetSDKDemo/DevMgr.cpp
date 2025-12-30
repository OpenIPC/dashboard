// DevMgr.cpp: implementation of the CDevMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "DevMgr.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Get CDevMgr object
//////////////////////////////////////////////////////////////////////
CDevMgr& CDevMgr::GetDevMgr(void)
{
	static CDevMgr devmgr;
	return devmgr;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDevMgr::CDevMgr()
{
	::InitializeCriticalSection(&m_csDev);
	::InitializeCriticalSection(&m_csAlarm);
	::InitializeCriticalSection(&m_csAlmList);
}

CDevMgr::~CDevMgr()
{
	list<DeviceNode*>::iterator it = m_lstDevice.begin();
	for(; it != m_lstDevice.end(); ++it)
	{
		if (*it)
		{
			delete (*it);
		}
	}

	list<AlarmNode*>::iterator it2 = m_lstAlarm.begin();
	for(; it2 != m_lstAlarm.end(); ++it2)
	{
		if (*it2)
		{
			delete (*it2);
		}
	}
	
	m_lstDevice.clear();
	m_lstAlarm.clear();

	::DeleteCriticalSection(&m_csDev);
	::DeleteCriticalSection(&m_csAlarm);
	::DeleteCriticalSection(&m_csAlmList);
}

void CDevMgr::For_EachDev(int (* cbForEach)(const DeviceNode& node, LDWORD dwUser), LDWORD dwUser)
{
	::EnterCriticalSection(&m_csDev);

	list<DeviceNode*>::iterator it = m_lstDevice.begin();
	for(; it != m_lstDevice.end(); ++it)
	{
		int nRet = cbForEach(**it, dwUser);
		if (1 == nRet)
		{
			break;
		}
	}
	
	::LeaveCriticalSection(&m_csDev);
}

void CDevMgr::For_EachAlmNode(int (* cbForEachAlm)(const AlarmNode& node, LDWORD dwUser), LDWORD dwUser)
{
	::EnterCriticalSection(&m_csAlmList);
	
	list<AlarmNode*>::iterator it = m_lstAlarm.begin();
	for(; it != m_lstAlarm.end(); ++it)
	{
		int nRet = cbForEachAlm(**it, dwUser);
		if (1 == nRet)
		{
			break;
		}
	}
	
	list<AlarmNode*>::iterator itdel = m_lstAlarm.begin();
	while(itdel != it)
	{
		if (*itdel) 
		{
			delete (*itdel);
			m_lstAlarm.erase(itdel++);
		}
	}

	::LeaveCriticalSection(&m_csAlmList);
}

int  CDevMgr::GetDev(LLONG lLoginID, DeviceNode& node)
{
	int nRet = -1;

	::EnterCriticalSection(&m_csDev);
		
	list<DeviceNode*>::iterator it = 
		find_if(m_lstDevice.begin(), m_lstDevice.end(), 
		CDevMgr::SearchDevByHandle(lLoginID));

	if (it != m_lstDevice.end())
	{
		memcpy(&node, *it, sizeof(DeviceNode));
		nRet = 0;
	}
	
	::LeaveCriticalSection(&m_csDev);

	return nRet;
}

int CDevMgr::SetAlarmInfo(LLONG lLoginID, LONG lCommand, char *pchDVRIP, LONG nDVRPort, char *pBuf, DWORD dwBufLen)
{
	int nRet = 0;
	
	::EnterCriticalSection(&m_csAlarm);
	
	list<DeviceNode*>::iterator it = 
		find_if(m_lstDevice.begin(), m_lstDevice.end(), 
		CDevMgr::SearchDevByHandle(lLoginID));
	
	AlarmNode *thisNode = new AlarmNode;
	if (!thisNode)
	{
		return -1;
	}
	memset(thisNode, 0, sizeof(AlarmNode));
	thisNode->timeStamp = CTime::GetCurrentTime();

	if (it != m_lstDevice.end())
	{
		thisNode->alarmType = lCommand;
		switch(lCommand)
		{
		case DH_COMM_ALARM:
			{
				if (dwBufLen != sizeof(NET_CLIENT_STATE))
				{
					nRet = -1;
					break;
				}
				NET_CLIENT_STATE *State = (NET_CLIENT_STATE *)pBuf;
				if(!State)
				{
					nRet = -1;
					break;
				}
				//设备列表中信息刷新
				(*it)->State.cState.channelcount = State->channelcount;
				(*it)->State.cState.alarminputcount = State->alarminputcount;
				memcpy((*it)->State.cState.alarm, State->alarm, 16);
				memcpy((*it)->State.cState.motiondection, State->motiondection, 16);
				memcpy((*it)->State.cState.videolost, State->videolost, 16);

				thisNode->alarmType = DH_COMM_ALARM;
				memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));

				nRet = 0;
			break;
			}
		case DH_SHELTER_ALARM:
			{
				if (dwBufLen != 16)
				{
					nRet = -1;
					break;
				}
				memcpy((*it)->State.shelter, pBuf, 16);
				
				thisNode->alarmType = DH_SHELTER_ALARM;
				memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));

				nRet = 0;
				break;
			}
		case DH_DISK_FULL_ALARM:
			{
				if (dwBufLen != sizeof(DWORD))
				{
					nRet = -1;
					break;
				}
				(*it)->State.dFull = *(DWORD*)pBuf;
				
				thisNode->alarmType = DH_DISK_FULL_ALARM;
				memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));

				nRet = 0;
				break;
			}
		case DH_DISK_ERROR_ALARM:
			{
				if (dwBufLen != sizeof(DWORD))
				{
					nRet = -1;
					break;
				}
				(*it)->State.dError = *(DWORD*)pBuf;
				
				thisNode->alarmType = DH_DISK_ERROR_ALARM;
				memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));

				nRet = 0;
				break;
			}
		case DH_SOUND_DETECT_ALARM:
			{
				if (dwBufLen != 16)
				{
					nRet = -1;
					break;
				}
				memcpy((*it)->State.soundalarm, pBuf, 16);
			
				thisNode->alarmType = DH_SOUND_DETECT_ALARM;
				memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));

				nRet = 0;
				break;
			}
		case DH_ALARM_DECODER_ALARM:
			{
				if (dwBufLen != 16)
				{
					nRet = -1;
					break;
				}
				memcpy((*it)->State.almDecoder, pBuf, 16);

				thisNode->alarmType = DH_ALARM_DECODER_ALARM;
				memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));
				nRet = 0;
			}
			break;
		case DH_REBOOT_EVENT_EX:
			{
				AfxMessageBox(ConvertString("Reboot?(y/n)"));
			}
			break;		
		default:
			nRet = -1;
			break;
		}
		memcpy(&thisNode->state, &(*it)->State, sizeof(DEV_STATE));
	}
	
	::LeaveCriticalSection(&m_csAlarm);
	
	if (nRet >= 0)
	{
		::EnterCriticalSection(&m_csAlmList);
		m_lstAlarm.push_back(thisNode);
		::LeaveCriticalSection(&m_csAlmList);
	}
	else
	{
		delete thisNode;
	}

	return nRet;
}

int CDevMgr::GetAlarmInfo(LLONG lLoginID, DEV_STATE *des)
{
	int nRet = -1;
	::EnterCriticalSection(&m_csAlarm);
	list<DeviceNode*>::iterator it = 
		find_if(m_lstDevice.begin(), m_lstDevice.end(), 
		CDevMgr::SearchDevByHandle(lLoginID));
	
	if (it != m_lstDevice.end())
	{
		memcpy(des, &(*it)->State, sizeof(DEV_STATE));
		nRet = 0;
	}
	::LeaveCriticalSection(&m_csAlarm);
	return nRet;
}

//add node to list
int  CDevMgr::PushBack(DeviceNode* node)
{
	if (!node)
	{
		return -1;
	}

	int nRet = 0;

	::EnterCriticalSection(&m_csDev);

	memset(&node->State, 0, sizeof(DEV_STATE));
	m_lstDevice.push_back(node);

	::LeaveCriticalSection(&m_csDev);

	return nRet;
}
	
//delete node from list
int  CDevMgr::DelNode(LLONG lLoginID)
{
	int nRet = -1;
	::EnterCriticalSection(&m_csDev);
	list<DeviceNode*>::iterator it = 
		find_if(m_lstDevice.begin(), m_lstDevice.end(), 
		SearchDevByHandle(lLoginID));
	
	if (it != m_lstDevice.end())
	{
		delete *it;
		m_lstDevice.erase(it);
		nRet = 0;
	}

	::LeaveCriticalSection(&m_csDev);

	return nRet;
}

int	 CDevMgr::ModifyNode(LLONG lLoginID, BOOL bIsOnline)
{
	int nRet = -1;
	::EnterCriticalSection(&m_csDev);
	list<DeviceNode*>::iterator it = 
		find_if(m_lstDevice.begin(), m_lstDevice.end(), 
		SearchDevByHandle(lLoginID));
	
	if (it != m_lstDevice.end())
	{
		DeviceNode *pDevNode = (*it);
		pDevNode->bIsOnline = bIsOnline;

		nRet = 0;
	}

	::LeaveCriticalSection(&m_csDev);

	return nRet;
}

BOOL CDevMgr::IsOnline(LLONG lLoginID)
{
	BOOL b = FALSE;
	::EnterCriticalSection(&m_csDev);

	list<DeviceNode*>::iterator it = 
		find_if(m_lstDevice.begin(), m_lstDevice.end(), 
		CDevMgr::SearchDevByHandle(lLoginID));

	if (it != m_lstDevice.end())
	{
		b = TRUE;
	}	

	::LeaveCriticalSection(&m_csDev);

	return b;
}

BOOL CDevMgr::IsOnline(DeviceNode* node)
{
	BOOL b = FALSE;
	::EnterCriticalSection(&m_csDev);

	list<DeviceNode*>::iterator it =
		find(m_lstDevice.begin(), m_lstDevice.end(), node);

	if (it != m_lstDevice.end())
	{
		b = TRUE;
	}

	::LeaveCriticalSection(&m_csDev);	

	return b;
}