#ifndef _DEVICE_H_
#define _DEVICE_H_

enum EM_NOTIFY_TYPE{
	EM_NOTIFY_DISCONNECT = 0,		
	EM_NOTIFY_RECONNECT,
	EM_NOTIFY_LOADPICTURE,
};

struct stuDeviceInfo {
	char szIp[32];
	int nPort;
	char szUserName[64];
	char szPasswd[64];
};

class CDeviceImpl;
class CObserver;
class CAlaram;

/************************************************************************/
/* Device Class Package device operation                                */
/************************************************************************/
class CDevice
{
public:
	static CDevice &GetInstance();

	void	AddObserver(CObserver* pObserver);

	void	DeleteObserver(CObserver* pObserver);

	bool	Login(const stuDeviceInfo& strInfo);

	bool	LogOut();

	bool	StartRealPlay(int nChannel, HWND hWnd);

	void	StopRealPlay();

	bool	StartRealLoadPicture(int nChannelID);

	void	StopRealLoadPicture();

	bool	GetParkingSpaceStatus(NET_IN_GET_PARKINGSPACE_STATUS* pstInParam, NET_OUT_GET_PARKINGSPACE_STATUS* pstOutParam);

	bool	SetParkingSpaceLightPlan(const NET_IN_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataIn,NET_OUT_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataOut);

	bool	GetParkingSpaceLightGroupInfo(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stOutParam);

	bool	SetParkingSpaceLightGroupInfo(const CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stInParam);

	bool	SetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stSetParam);

	bool	GetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stGetParam);

	bool	isOnline();

	int		GetChannelNum() const;

private:
	CDevice();
	~CDevice();
	CDevice(const CDevice& device);
	CDevice& operator=(const CDevice& device);

private:
	CDeviceImpl*	m_pDeviceImpl;
};

#endif