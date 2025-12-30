#ifndef _ATTENDANCE_MANAGER_H_
#define _ATTENDANCE_MANAGER_H_
#include  "../../../Include/Common/dhnetsdk.h"

class CAttendanceObserver;
class CAttendanceManagerImpl;

class CAttendanceManager
{
public:
	CAttendanceManager();
	~CAttendanceManager();

	// get last error
	DWORD GetLastError();

	// true:success false:failed 
	bool LoginDevice(const char *szIP, unsigned short nPort, const char *szUser, const char *szPasswd, int *pError);

	// true:success false:failed
	bool LogoutDevice();

	// true:success false:failed 
	// nChannel:channel id dwAlarmType: alarm type CAttendanceObserver:callback userinfo
	bool RealLoadPicture(int nChannel, DWORD dwAlarmType, CAttendanceObserver *pUser);

	//true:success false:failed
	bool StopLoadPicture();

	// add attendance user
	bool AddAttendanceUser(NET_IN_ATTENDANCE_ADDUSER *pstuInAddUser);

	// modify attendance user
	bool ModifyAttendanceUser(NET_IN_ATTENDANCE_ModifyUSER *pstuInModifyUser);

	// delete attendance user
	bool DelAttendanceUser(NET_IN_ATTENDANCE_DELUSER *pstuInDelUser);

	// get attendance user
	bool GetAttendanceUser(NET_IN_ATTENDANCE_GetUSER *pstuInGetUser, NET_OUT_ATTENDANCE_GetUSER *pstuOutGetUser);
	
	// find attendance user
	bool FindAttendanceUser(NET_IN_ATTENDANCE_FINDUSER *pstuInFindUser, NET_OUT_ATTENDANCE_FINDUSER *psutOutFindUser); 

	// start listenEx
	bool StartListenEx();

	// stop listen
	bool StopListen();

	// attendance get finger by userid
	bool GetFingerByUserID(NET_IN_FINGERPRINT_GETBYUSER *pstuIn, NET_OUT_FINGERPRINT_GETBYUSER *pstuOut);
	
	// attendance insert finger by userid
	bool InsertFingerByUserID(NET_IN_FINGERPRINT_INSERT_BY_USERID* pstuInInsert, NET_OUT_FINGERPRINT_INSERT_BY_USERID* pstuOutInsert);

	// attendance remove finger by userid
	bool RemoveFingerByUserID(NET_CTRL_IN_FINGERPRINT_REMOVE_BY_USERID* pstuInRemove, NET_CTRL_OUT_FINGERPRINT_REMOVE_BY_USERID* pstuOutRemove);

	// capture finger print
	bool CaptureFingerprint(); 

	// get finger record by fingerid
	bool GetFingerRecord(NET_CTRL_IN_FINGERPRINT_GET*pstuInGet, NET_CTRL_OUT_FINGERPRINT_GET* pstuOutGet);

	// remove finger record by fingerid
	bool RemoveFingerRecord(NET_CTRL_IN_FINGERPRINT_REMOVE*  pstuInRemove, NET_CTRL_OUT_FINGERPRINT_REMOVE* pstuOutRemove);

	void SetDVRMessCallBack(fMessCallBack cbMessage,LDWORD dwUser);
private:
	CAttendanceManagerImpl *m_pImpl;
};
#endif