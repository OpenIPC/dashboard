#include "StdAfx.h"
#include "Device.h"
#include "Alaram.h"
#include "Observer.h"
#include "Observable.h"


struct stuBussinessInfo{
	int		nChannel;
	LLONG	lHandle;
};

class CDeviceImpl : public CObservable
{
public:
	CDeviceImpl();
	~CDeviceImpl();

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

	int		GetChannelNum() const;

	bool	isOnline();

private:
	LLONG	m_lLoginID;
	int		m_nChannelNum;

	std::vector<stuBussinessInfo>	m_vecReaPlaylHandle;	
	std::vector<stuBussinessInfo>	m_vecRealLoadHandle;
};

void CALLBACK DevReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	CDeviceImpl* pThis = (CDeviceImpl*)dwUser;
	if (NULL == pThis)
	{
		return ;
	}

	pThis->Notify(EM_NOTIFY_RECONNECT, NULL);
	return;
}


void CALLBACK DevDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	CDeviceImpl* pThis = (CDeviceImpl*)dwUser;
	if (NULL == pThis)
	{
		return ;
	}

	pThis->Notify(EM_NOTIFY_DISCONNECT, NULL);
	return;
}

int CALLBACK RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata)
{
	CDeviceImpl* pThis = (CDeviceImpl*)dwUser;
	if (NULL == pThis)
	{
		return 0;
	}

	CAlaram *pAlarm = NEW CAlaram;
	if (NULL == pAlarm)
	{
		return 0;
	}

	bool bRet = pAlarm->SetAlarmData(lAnalyzerHandle, dwAlarmType, pAlarmInfo, pBuffer, dwBufSize, nSequence);
	if (false == bRet)
	{
		delete pAlarm;
		return 0;
	}
	
	pThis->Notify(EM_NOTIFY_LOADPICTURE, (void*)pAlarm);

	return 0;
}

CDeviceImpl::CDeviceImpl():m_lLoginID(0), m_nChannelNum(0)
{
	(void)CLIENT_Init(&DevDisConnect, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(&DevReConnect, (LDWORD)this);

	LOG_SET_PRINT_INFO logInfo;
	memset(&logInfo, 0, sizeof(logInfo));
	(void)CLIENT_LogOpen(&logInfo);
}

CDeviceImpl::~CDeviceImpl()
{
	StopRealLoadPicture();
	StopRealPlay();
	LogOut();

	CLIENT_Cleanup();
}

bool	CDeviceImpl::Login(const stuDeviceInfo& strInfo)
{
	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, strInfo.szIp, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, strInfo.szPasswd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, strInfo.szUserName, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = strInfo.nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lLoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (lLoginID == 0)
	{
		return false;
	}
	m_nChannelNum = stOutparam.stuDeviceInfo.nChanNum;
	m_lLoginID = lLoginID;
	return true;
}

bool	CDeviceImpl::LogOut()
{
	m_nChannelNum = 0;
	BOOL bRet = CLIENT_Logout(m_lLoginID);
	m_lLoginID = 0;
	return (bRet==FALSE?false:true);
}

bool	CDeviceImpl::StartRealPlay(int nChannel, HWND hWnd)
{
	LLONG lRet = CLIENT_RealPlayEx(m_lLoginID, nChannel, hWnd);
	if (lRet == 0)
	{
		return false;
	}
	stuBussinessInfo info = {nChannel, lRet};
	m_vecReaPlaylHandle.push_back(info);
	return true;
}

void	CDeviceImpl::StopRealPlay()
{
	for (int i = 0; i < m_vecReaPlaylHandle.size(); i++)
	{
		if (m_vecReaPlaylHandle[i].lHandle)
		{
			CLIENT_StopRealPlay(m_vecReaPlaylHandle[i].lHandle);
		}
	}
	m_vecReaPlaylHandle.clear();
}

bool	CDeviceImpl::StartRealLoadPicture(int nChannelID)
{
	LLONG lRet = CLIENT_RealLoadPictureEx(m_lLoginID, nChannelID, EVENT_IVS_ALL, TRUE, RealLoadPicCallback, (LDWORD)(this), NULL);
	if (lRet == 0)
	{
		return false;
	}
	stuBussinessInfo info = {nChannelID, lRet};
	m_vecRealLoadHandle.push_back(info);
	return true;
}

void	CDeviceImpl::StopRealLoadPicture()
{
	for(int i = 0; i < m_vecRealLoadHandle.size(); i++)
	{
		if (m_vecRealLoadHandle[i].lHandle)
		{
			CLIENT_StopLoadPic(m_vecRealLoadHandle[i].lHandle);
		}
	}
	m_vecRealLoadHandle.clear();
}

bool	CDeviceImpl::GetParkingSpaceStatus(NET_IN_GET_PARKINGSPACE_STATUS* pstInParam, NET_OUT_GET_PARKINGSPACE_STATUS* pstOutParam)
{
	BOOL bRet = CLIENT_GetParkingSpaceStatus(m_lLoginID, pstInParam, pstOutParam);
	return bRet==FALSE?false:true;
}

bool	CDeviceImpl::SetParkingSpaceLightPlan(const NET_IN_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataIn,NET_OUT_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataOut)
{
	BOOL bRet = CLIENT_SetParkingSpaceLightPlan(m_lLoginID, pNetDataIn, pNetDataOut, 5000);
	return bRet==FALSE?false:true;
}

bool	CDeviceImpl::GetParkingSpaceLightGroupInfo(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stOutParam)
{
	int nError = 0;
	char szJsonBuf[1024 * 50];
	memset(szJsonBuf, 0, sizeof(szJsonBuf));
	int nChannelID = -1;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_PARKING_SPACE_LIGHT_GROUP,nChannelID,szJsonBuf,1024 * 50,&nError,MAX_TIMEOUT);
	if (bRet == FALSE)
	{
		return false;
	}

	DWORD dwRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_PARKING_SPACE_LIGHT_GROUP,szJsonBuf,(char*)stOutParam,sizeof(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL),&dwRetLen);
	if (!bRet)
	{
		return false;
	}

	return true;
}

bool	CDeviceImpl::SetParkingSpaceLightGroupInfo(const CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stInParam)
{
	char szJsonBuf[1024 * 50];
	memset(szJsonBuf, 0, sizeof(szJsonBuf));

	BOOL bRet = CLIENT_PacketData(CFG_CMD_PARKING_SPACE_LIGHT_GROUP,(LPVOID)stInParam, sizeof(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL), szJsonBuf, sizeof(szJsonBuf));
	if (bRet == FALSE)
	{
		return false;
	}
	int nerror = 0;
	int nrestart = 0;
	int nChannelID = -1;
	bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_PARKING_SPACE_LIGHT_GROUP, nChannelID, szJsonBuf, 1024 * 50, &nerror, &nrestart, 3000);
	if(bRet == FALSE)
	{
		return false;
	}
	return true;
}

bool	CDeviceImpl::SetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stSetParam)
{
	BOOL bRet = CLIENT_SetConfig(m_lLoginID, NET_EM_CFG_PARKINGSPACELIGHT_STATE, -1, stSetParam, sizeof(NET_PARKINGSPACELIGHT_STATE_INFO));
	return bRet==FALSE?false:true;
}

bool	CDeviceImpl::GetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stGetParam)
{
	BOOL bRet = CLIENT_GetConfig(m_lLoginID, NET_EM_CFG_PARKINGSPACELIGHT_STATE, -1, stGetParam, sizeof(NET_PARKINGSPACELIGHT_STATE_INFO));
	return bRet==FALSE?false:true;
}

int		CDeviceImpl::GetChannelNum() const
{
	return m_nChannelNum;
}

bool	CDeviceImpl::isOnline()
{
	return m_lLoginID==0? false:true;
}

CDevice &CDevice::GetInstance()
{
	static CDevice instance;
	return instance;
}

CDevice::CDevice()
{
	m_pDeviceImpl = NEW CDeviceImpl;
}

CDevice::~CDevice()
{
	if (m_pDeviceImpl)
	{
		delete m_pDeviceImpl;
		m_pDeviceImpl = NULL;
	}
}

bool	CDevice::Login(const stuDeviceInfo& strInfo)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->Login(strInfo);
	}
	return false;
}

bool	CDevice::LogOut()
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->LogOut();
	}
	return false;
}

bool	CDevice::StartRealPlay(int nChannel, HWND hWnd)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->StartRealPlay(nChannel, hWnd);
	}
	return false;
}

void	CDevice::StopRealPlay()
{
	if (m_pDeviceImpl)
	{
		m_pDeviceImpl->StopRealPlay();
	}
}

bool	CDevice::StartRealLoadPicture(int nChannelID/*, void* cbAnalyzerData, void* dwUser*/)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->StartRealLoadPicture(nChannelID/*, cbAnalyzerData, dwUser*/);
	}
	return false;
}

void	CDevice::StopRealLoadPicture()
{
	if (m_pDeviceImpl)
	{
		m_pDeviceImpl->StopRealLoadPicture();
	}
}

int		CDevice::GetChannelNum() const
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->GetChannelNum();
	}
	return 0;
}

bool	CDevice::isOnline()
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->isOnline();
	}
	return false;
}

bool	CDevice::GetParkingSpaceStatus(NET_IN_GET_PARKINGSPACE_STATUS* pstInParam, NET_OUT_GET_PARKINGSPACE_STATUS* pstOutParam)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->GetParkingSpaceStatus(pstInParam, pstOutParam);
	}
	return false;
}

bool	CDevice::SetParkingSpaceLightPlan(const NET_IN_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataIn,NET_OUT_SET_PARKING_SPACE_LIGHT_PLAN* pNetDataOut)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->SetParkingSpaceLightPlan(pNetDataIn, pNetDataOut);
	}
	return false;
}

bool	CDevice::GetParkingSpaceLightGroupInfo(CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stOutParam)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->GetParkingSpaceLightGroupInfo(stOutParam);
	}
	return false;
}

bool	CDevice::SetParkingSpaceLightGroupInfo(const CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL* stInParam)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->SetParkingSpaceLightGroupInfo(stInParam);
	}
	return false;
}

bool	CDevice::SetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stSetParam)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->SetParkingSpaceLightState(stSetParam);
	}
	return false;
}

bool	CDevice::GetParkingSpaceLightState(NET_PARKINGSPACELIGHT_STATE_INFO* stGetParam)
{
	if (m_pDeviceImpl)
	{
		return m_pDeviceImpl->GetParkingSpaceLightState(stGetParam);
	}
	return false;
}

void	CDevice::AddObserver(CObserver* pObserver)
{
	if (m_pDeviceImpl)
	{
		m_pDeviceImpl->AddObserver(pObserver);
	}
}

void	CDevice::DeleteObserver(CObserver* pObserver)
{
	if (m_pDeviceImpl)
	{
		m_pDeviceImpl->DeleteObserver(pObserver);
	}
}