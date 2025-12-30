#pragma once

/************************************************************************/
/*        Control class, Responsible for interface and module interaction */
/************************************************************************/
struct stuDeviceInfo;
class CObserver;
class CAlaram;
class CControllerImpl;
class CController
{
public:
	explicit CController(CObserver *pView);
	~CController(void);

	bool	Login(const stuDeviceInfo& strInfo);

	bool	LogOut();

	bool	StartRealPlay(int nChannel, HWND hWnd);
	
	void	StopRealPlay();

	bool	StartRealLoadPicture(int nChannelID);

	void	StopRealLoadPicture();

	// Get parking status information
	bool	GetParkingSpaceStatus(NET_IN_GET_PARKINGSPACE_STATUS* pstInParam, NET_OUT_GET_PARKINGSPACE_STATUS* pstOutParam);

	// Set parking light plan
	bool	SetParkingSpaceLightPlan(const NET_IN_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataIn,NET_OUT_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataOut);

	// Get the local configuration of parking space indicator
	bool	GetParkingSpaceLightGroupInfo(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stOutParam);

	// Set parking position indicator light local configuration
	bool	SetParkingSpaceLightGroupInfo(const CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stInParam);

	// Set parking light status
	bool	SetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stSetParam);

	// Get parking light status
	bool	GetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stGetParam);

	bool	isOnline();

	int		GetChannelNum() const;

private:
	CControllerImpl*	m_pControlImpl;
};
