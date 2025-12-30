#include "stdafx.h"
#include "AttendanceManager.h"
#include "CAttendanceObserver.h"
#include "AttendanceManagerImpl.h"

#if defined(_WIN64)
#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
#else
#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
#endif

CAttendanceManager::CAttendanceManager()
{
	m_pImpl = new CAttendanceManagerImpl;
	if (m_pImpl)
	{
		CLIENT_Init(NULL, NULL);
		LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
		CLIENT_LogOpen(&stLogPrintInfo);
	}
}

CAttendanceManager::~CAttendanceManager()
{
	if (m_pImpl)
	{
		LogoutDevice();

		CLIENT_Cleanup();

		delete m_pImpl;
		m_pImpl = NULL;
	}
}

// get last error
DWORD CAttendanceManager::GetLastError()
{
	DWORD dwError = CLIENT_GetLastError();
	return dwError;
}

// true:success false:failed 
bool CAttendanceManager::LoginDevice(const char *szIP, unsigned short nPort, const char *szUser, const char *szPasswd, int *pError)
{
	if (NULL == m_pImpl)
	{
		return false;
	}

	if (m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, szIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, szPasswd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, szUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lLoginHandle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (lLoginHandle)
	{
		m_pImpl->m_lDeviceHandle = lLoginHandle;
		return true;
	}
	else
	{
		return false;
	}
}

// true:success false:failed
bool CAttendanceManager::LogoutDevice()
{
	if (NULL == m_pImpl)
	{
		return false;
	}

	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;	
	}
	
	if (m_pImpl->m_lRealLoadHandle)
	{
		CLIENT_StopLoadPic(m_pImpl->m_lRealLoadHandle);
		m_pImpl->m_lRealLoadHandle = 0;
	}
	BOOL bRet = CLIENT_Logout(m_pImpl->m_lDeviceHandle);
	m_pImpl->m_lDeviceHandle = 0;
	return bRet==TRUE ? true:false;
}

int  CALLBACK funcAnalyzerDataCallBack(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
	CAttendanceObserver *pObserver = (CAttendanceObserver *)dwUser;
	if (NULL == pObserver)
	{
		return -1;
	}

	switch(dwAlarmType)
	{
	case EVENT_IVS_ACCESS_CTL:
		{
			pObserver->updateAttendanceInfo(ATTENDANCE_MSG_TYPE_IVS_ACCESS_CTL, pAlarmInfo, pBuffer, dwBufSize);
		}
		break;
	default:
		break;
	}

	return 0;
}

// true:success false:failed 
bool CAttendanceManager::RealLoadPicture(int nChannel, DWORD dwAlarmType, CAttendanceObserver *pUser)
{
	if (NULL == m_pImpl)
	{
		return false;
	}

	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	if (m_pImpl->m_lRealLoadHandle)
	{
		return false;
	}

	LLONG lRealLoadPic = CLIENT_RealLoadPictureEx(m_pImpl->m_lDeviceHandle, nChannel, dwAlarmType, TRUE, funcAnalyzerDataCallBack, (LDWORD)pUser, NULL);
	if (lRealLoadPic)
	{
		m_pImpl->m_lRealLoadHandle = lRealLoadPic;
		return true;
	}
	else
	{
		return false;
	}
}

//true:success false:failed
bool CAttendanceManager::StopLoadPicture()
{
	if (NULL == m_pImpl)
	{
		return false;
	}

	if (0 == m_pImpl->m_lDeviceHandle || 0 == m_pImpl->m_lRealLoadHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_StopLoadPic(m_pImpl->m_lRealLoadHandle);
	m_pImpl->m_lRealLoadHandle = 0;
	return bRet==TRUE ? true: false;
}

// add attendance user
bool CAttendanceManager::AddAttendanceUser(NET_IN_ATTENDANCE_ADDUSER *pstuInAddUser)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	NET_OUT_ATTENDANCE_ADDUSER stuOutAddUser = {sizeof(stuOutAddUser)};
	BOOL bRet = CLIENT_Attendance_AddUser(m_pImpl->m_lDeviceHandle, pstuInAddUser, &stuOutAddUser, 5000);
	return bRet == TRUE ? true:false;
}

// modify attendance user
bool CAttendanceManager::ModifyAttendanceUser(NET_IN_ATTENDANCE_ModifyUSER *pstuInModifyUser)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	NET_OUT_ATTENDANCE_ModifyUSER ModifyUser = {sizeof(ModifyUser)};
	BOOL bRet = CLIENT_Attendance_ModifyUser(m_pImpl->m_lDeviceHandle, pstuInModifyUser, &ModifyUser, 5000);
	return bRet == TRUE ? true:false;
}

// delete attendance user
bool CAttendanceManager::DelAttendanceUser(NET_IN_ATTENDANCE_DELUSER *pstuInDelUser)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	NET_OUT_ATTENDANCE_DELUSER stuDelUser = {sizeof(stuDelUser)};
	BOOL bRet = CLIENT_Attendance_DelUser(m_pImpl->m_lDeviceHandle, pstuInDelUser, &stuDelUser, 5000);
	return bRet == TRUE ? true:false;
}

// get attendance user
bool CAttendanceManager::GetAttendanceUser(NET_IN_ATTENDANCE_GetUSER *pstuInGetUser, NET_OUT_ATTENDANCE_GetUSER *pstuOutGetUser)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_Attendance_GetUser(m_pImpl->m_lDeviceHandle, pstuInGetUser, pstuOutGetUser, 5000);
	return bRet == TRUE ? true:false;
}

// find attendance user
bool CAttendanceManager::FindAttendanceUser(NET_IN_ATTENDANCE_FINDUSER *pstuInFindUser, NET_OUT_ATTENDANCE_FINDUSER *psutOutFindUser)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_Attendance_FindUser(m_pImpl->m_lDeviceHandle, pstuInFindUser, psutOutFindUser, 5000);
	return bRet == TRUE ? true:false;	
}

// start listenEx
bool CAttendanceManager::StartListenEx()
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_StartListenEx(m_pImpl->m_lDeviceHandle);

	return bRet == TRUE ? true:false;
}

// stop listen
bool CAttendanceManager::StopListen()
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_StopListen(m_pImpl->m_lDeviceHandle);

	return bRet == TRUE ? true:false;
}

// attendance get finger by userid
bool CAttendanceManager::GetFingerByUserID(NET_IN_FINGERPRINT_GETBYUSER *pstuIn, NET_OUT_FINGERPRINT_GETBYUSER *pstuOut)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_Attendance_GetFingerByUserID(m_pImpl->m_lDeviceHandle,pstuIn, pstuOut, 10000);

	return bRet == TRUE ? true:false;
}

// attendance insert finger by userid
bool CAttendanceManager::InsertFingerByUserID(NET_IN_FINGERPRINT_INSERT_BY_USERID* pstuInInsert, NET_OUT_FINGERPRINT_INSERT_BY_USERID* pstuOutInsert)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_Attendance_InsertFingerByUserID(m_pImpl->m_lDeviceHandle, pstuInInsert,pstuOutInsert, 5000);

	return bRet == TRUE ? true:false;
}

// attendance remove finger by userid
bool CAttendanceManager::RemoveFingerByUserID(NET_CTRL_IN_FINGERPRINT_REMOVE_BY_USERID* pstuInRemove, NET_CTRL_OUT_FINGERPRINT_REMOVE_BY_USERID* pstuOutRemove)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}
	
	BOOL bRet = CLIENT_Attendance_RemoveFingerByUserID(m_pImpl->m_lDeviceHandle,pstuInRemove,pstuOutRemove, 5000);

	return bRet == TRUE ? true:false;
}

// capture finger print
bool CAttendanceManager::CaptureFingerprint()
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	NET_CTRL_CAPTURE_FINGER_PRINT stuCapParam = {sizeof(stuCapParam)};
	stuCapParam.nChannelID = 0;
	strncpy(stuCapParam.szReaderID, "1", sizeof(stuCapParam.szReaderID)-1);
	BOOL bRet = CLIENT_ControlDevice(m_pImpl->m_lDeviceHandle, DH_CTRL_CAPTURE_FINGER_PRINT, &stuCapParam, 5000);
	return bRet == TRUE ? true:false;
}

// get finger record by fingerid
bool CAttendanceManager::GetFingerRecord(NET_CTRL_IN_FINGERPRINT_GET*pstuInGet, NET_CTRL_OUT_FINGERPRINT_GET* pstuOutGet)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_Attendance_GetFingerRecord(m_pImpl->m_lDeviceHandle,pstuInGet, pstuOutGet, 5000);
	return bRet == TRUE ? true:false;
}

// remove finger record by fingerid
bool CAttendanceManager::RemoveFingerRecord(NET_CTRL_IN_FINGERPRINT_REMOVE*  pstuInRemove, NET_CTRL_OUT_FINGERPRINT_REMOVE* pstuOutRemove)
{
	if (NULL == m_pImpl)
	{
		return false;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return false;
	}

	BOOL bRet = CLIENT_Attendance_RemoveFingerRecord(m_pImpl->m_lDeviceHandle,pstuInRemove,pstuOutRemove, 5000);

	return bRet == TRUE ? true:false;
}

void CAttendanceManager::SetDVRMessCallBack(fMessCallBack cbMessage,LDWORD dwUser)
{
	if (NULL == m_pImpl)
	{
		return ;
	}
	if (0 == m_pImpl->m_lDeviceHandle)
	{
		return ;
	}

	CLIENT_SetDVRMessCallBack(cbMessage, dwUser);

	return ;
}

