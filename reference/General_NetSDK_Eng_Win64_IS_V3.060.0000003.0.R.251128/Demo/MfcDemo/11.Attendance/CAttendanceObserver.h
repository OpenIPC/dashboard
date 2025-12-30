#ifndef _ATTENDANCE_OBSERVER_H_
#define _ATTENDANCE_OBSERVER_H_
#include  "../../../Include/Common/dhnetsdk.h"

typedef enum tagATTENDANCE_MSG_TYPE
{
	ATTENDANCE_MSG_TYPE_UNKNOWN,             // 未知
	ATTENDANCE_MSG_TYPE_IVS_ACCESS_CTL,      // 智能门禁事件  对应结构体DEV_EVENT_ACCESS_CTL_INFO
}ATTENDANCE_MSG_TYPE;

//观察者  如果有谁关心消息，需要继承这个类
class CAttendanceObserver
{
public:
	CAttendanceObserver();
	virtual ~CAttendanceObserver();

	// 更新接口  
	/*
		msgType           消息类型
		void *pMsgInfo    消息对应结构体
		BYTE *pBuffer     图片数据等，可能为空
		DWORD dwBufSize   buffer大小，可能不用
	*/
	virtual int updateAttendanceInfo(ATTENDANCE_MSG_TYPE msgType, void *pMsgInfo, BYTE *pBuffer, DWORD dwBufSize);
};



#endif