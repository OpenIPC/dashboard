#include "stdafx.h"
#include "Controller.h"
#include "Device.h"
#include "ParkingSpaceDetectorDlg.h"
#include "Alaram.h"


class CControllerImpl
{
public:
	CControllerImpl(CObserver *pView);
	~CControllerImpl(void);

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
	CObserver* m_pView;
};

CControllerImpl::CControllerImpl(CObserver *pView):m_pView(pView)
{
	CDevice::GetInstance().AddObserver((CObserver*)m_pView);
}

CControllerImpl::~CControllerImpl(void)
{
	CDevice::GetInstance().DeleteObserver((CObserver*)m_pView);
}

bool	CControllerImpl::Login(const stuDeviceInfo& strInfo)
{
	return CDevice::GetInstance().Login(strInfo);
}

bool	CControllerImpl::LogOut()
{
	return CDevice::GetInstance().LogOut();
}

bool	CControllerImpl::StartRealPlay(int nChannel, HWND hWnd)
{
	return CDevice::GetInstance().StartRealPlay(nChannel, hWnd);
}

void	CControllerImpl::StopRealPlay()
{
	CDevice::GetInstance().StopRealPlay();
}

bool	CControllerImpl::StartRealLoadPicture(int nChannelID)
{
	return CDevice::GetInstance().StartRealLoadPicture(nChannelID);
}

void	CControllerImpl::StopRealLoadPicture()
{
	CDevice::GetInstance().StopRealLoadPicture();
}

bool	CControllerImpl::GetParkingSpaceStatus(NET_IN_GET_PARKINGSPACE_STATUS* pstInParam, NET_OUT_GET_PARKINGSPACE_STATUS* pstOutParam)
{
	return CDevice::GetInstance().GetParkingSpaceStatus(pstInParam, pstOutParam);
}

bool	CControllerImpl::SetParkingSpaceLightPlan(const NET_IN_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataIn,NET_OUT_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataOut)
{
	return CDevice::GetInstance().SetParkingSpaceLightPlan(pNetDataIn, pNetDataOut);
}

bool	CControllerImpl::GetParkingSpaceLightGroupInfo(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stOutParam)
{
	return CDevice::GetInstance().GetParkingSpaceLightGroupInfo(stOutParam);
}

bool	CControllerImpl::SetParkingSpaceLightGroupInfo(const CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stInParam)
{
	return CDevice::GetInstance().SetParkingSpaceLightGroupInfo(stInParam);
}

bool	CControllerImpl::SetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stSetParam)
{
	return CDevice::GetInstance().SetParkingSpaceLightState(stSetParam);
}

bool	CControllerImpl::GetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stGetParam)
{
	return CDevice::GetInstance().GetParkingSpaceLightState(stGetParam);
}

bool	CControllerImpl::isOnline()
{
	return CDevice::GetInstance().isOnline();
}

int		CControllerImpl::GetChannelNum() const
{
	return CDevice::GetInstance().GetChannelNum();
}


CController::CController(CObserver *pView)
{
	m_pControlImpl = new CControllerImpl(pView);
}

CController::~CController(void)
{
	if (m_pControlImpl)
	{
		delete m_pControlImpl;
		m_pControlImpl = NULL;
	}
}

bool	CController::Login(const stuDeviceInfo& strInfo)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->Login(strInfo);
	}
	return false;
}

bool	CController::LogOut()
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->LogOut();
	}
	return false;
}

bool	CController::StartRealPlay(int nChannel, HWND hWnd)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->StartRealPlay(nChannel, hWnd);
	}
	return false;
}

void	CController::StopRealPlay()
{
	if (m_pControlImpl)
	{
		m_pControlImpl->StopRealPlay();
	}
}

bool	CController::StartRealLoadPicture(int nChannelID)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->StartRealLoadPicture(nChannelID);
	}
	return false;
}

void	CController::StopRealLoadPicture()
{
	if (m_pControlImpl)
	{
		m_pControlImpl->StopRealLoadPicture();
	}
}

bool	CController::GetParkingSpaceStatus(NET_IN_GET_PARKINGSPACE_STATUS* pstInParam, NET_OUT_GET_PARKINGSPACE_STATUS* pstOutParam)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->GetParkingSpaceStatus(pstInParam, pstOutParam);
	}
	return false;
}

bool	CController::SetParkingSpaceLightPlan(const NET_IN_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataIn,NET_OUT_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataOut)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->SetParkingSpaceLightPlan(pNetDataIn, pNetDataOut);
	}
	return false;
}

bool	CController::GetParkingSpaceLightGroupInfo(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stOutParam)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->GetParkingSpaceLightGroupInfo(stOutParam);
	}
	return false;
}

bool	CController::SetParkingSpaceLightGroupInfo(const CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stInParam)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->SetParkingSpaceLightGroupInfo(stInParam);
	}
	return false;
}

bool	CController::SetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stSetParam)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->SetParkingSpaceLightState(stSetParam);
	}
	return false;
}

bool	CController::GetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stGetParam)
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->GetParkingSpaceLightState(stGetParam);
	}
	return false;
}

bool	CController::isOnline()
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->isOnline();
	}
	return false;
}

int		CController::GetChannelNum() const
{
	if (m_pControlImpl)
	{
		return m_pControlImpl->GetChannelNum();
	}
	return 0;
}
