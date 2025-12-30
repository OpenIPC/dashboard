// DlgCapability.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCapability.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCapability dialog

CDlgCapability::CDlgCapability(CWnd* pParent /* = NULL */, LLONG hLoginID /* = NULL */, AV_uint32 uiAlarmIn /* = 0 */, AV_uint32 uiAlarmOut /* = 0 */)
	: CDialog(CDlgCapability::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCapability)
	m_hLogin = hLoginID;
	m_uiAlarmIn = uiAlarmIn;
	m_uiAlarmOut = uiAlarmOut;
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgCapability::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCapability)
	DDX_Control(pDX, IDC_DLG_CAP_LIST_ERR, m_lsErrorInfo);
	DDX_Control(pDX, IDC_DLG_CAP_EDT_SHOW, m_edtShow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCapability, CDialog)
	//{{AFX_MSG_MAP(CDlgCapability)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCapability message handlers

BOOL CDlgCapability::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CAPABILITY);
	// TODO: Add extra initialization here
	if (!m_hLogin)
	{
		OutputInfo(ConvertString(CString("we haven't login a device yet!"), DLG_CAPABILITY));
		return TRUE;
	}

	GetCapability();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCapability::GetCapability()
{
	// 显示能力信息
	CString csCap;

	// 普通(本地)报警输入输出通道数
	csCap.Format("%s: %d\r\n%s: %d\r\n\r\n", 
		ConvertString("AlarmIn", DLG_CAPABILITY),
		m_uiAlarmIn, 
		ConvertString("AlarmOut", DLG_CAPABILITY),
		m_uiAlarmOut);
	
	// 报警键盘信息
	CString csAlarmKeyboard;
	if (GetAlarmKeyboardOnlineCount(csAlarmKeyboard))
	{
		csCap += csAlarmKeyboard;
	}
	
	// 文件管理能力
	CString csFileManager;
	if (GetFileManagerCap(csFileManager))
	{
		csCap += csFileManager;
	}

	// 日志服务能力
	CString csLogService;
	if (GetLogServiceCap(csLogService))
	{
		csCap += csLogService;
	}

    // 获取开关量报警能力
	CString csAlarmIn;
	if (GetAlarmInCap(csAlarmIn))
	{
		csCap += csAlarmIn;
	}			

	// 获取设备联动能力
	CString csEventHandler;
	if (GetEventHandlerCap(csEventHandler))
	{
		csCap += csEventHandler;
	}

	// 获取扩展模块报警盒能力
	CString csExAlarmBox;
	if (GetExAlarmBoxCap(csExAlarmBox))
	{
		csCap += csExAlarmBox;
	}

	// 获取扩展模块报警能力
	CString csExAlarm;
	if (GetExAlarmInCap(csExAlarm))
	{
		csCap += csExAlarm;
	}

	// 门禁能力
	CString csAccessControlCap;
	if (GetAccessControlCap(csAccessControlCap))
	{
		csCap += csAccessControlCap;
	}

	// 模拟量报警通道能力
	CString csAnalogAlarmCap;
	if (GetAnalogAlarmCap(csAnalogAlarmCap))
	{
		csCap += csAnalogAlarmCap;
	}

    // 低速私域网能力
    CString csLowRateWPanCap;
    if (GetLowRateWPanCap(csLowRateWPanCap))
    {
        csCap += csLowRateWPanCap;
    }

	// 界面显示能力结果
	{		
		m_edtShow.SetWindowText(csCap);
	}
}

void CDlgCapability::OutputInfo(const CString& csInfo)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	CString csOut;
	csOut.Format("%04d-%02d-%02d %02d:%02d:%02d: %s",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
		csInfo);
	
	m_lsErrorInfo.InsertString(0, csOut);
}

BOOL CDlgCapability::GetAlarmKeyboardOnlineCount(CString& csInfo)
{	
	NET_ALARMKEYBOARD_COUNT stuInfo = {sizeof(stuInfo)};
	int nRet = 0;
	BOOL bRet = CLIENT_QueryDevState(m_hLogin, DH_DEVSTATE_ALARMKEYBOARD_COUNT, (char*)&stuInfo, sizeof(stuInfo), &nRet, SDK_API_WAIT);
	if (bRet && nRet >= sizeof(stuInfo))
	{
		CString csAlarmKeyboard;
		csAlarmKeyboard.Format("%s = %d\r\n",
			ConvertString("Online AlarmKeyboard Count", DLG_CAPABILITY),
			stuInfo.nAlarmKeyboardCount);
		csInfo += csAlarmKeyboard;
		csInfo += "\r\n";
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetFileManagerCap(CString& csCap)
{
	char szBuff[1024] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_FILEMANAGER, 0, szBuff, sizeof(szBuff), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_FILEMANAGER stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_FILEMANAGER, szBuff, &stuInfo, sizeof(CFG_CAP_FILEMANAGER), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_FILEMANAGER))
		{
			CString csFileManeger;
			csFileManeger.Format("%s = %d %s\r\n", 
				ConvertString("Max size of upload file", DLG_CAPABILITY),
				stuInfo.nMaxUploadFileSize,
				ConvertString("bytes", DLG_CAPABILITY));
			csCap += csFileManeger;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetLogServiceCap(CString& csCap)
{
	char szBuff[1024] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_LOG, 0, szBuff, sizeof(szBuff), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_LOG stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_LOG, szBuff, &stuInfo, sizeof(CFG_CAP_LOG), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_LOG))
		{
			CString csLogServiceCap;
			csLogServiceCap.Format("%s = %d\r\n\
%s = %d\r\n\
%s = %s\r\n\
%s = %s\r\n\
%s = %s\r\n",
				ConvertString("LogMaxItem", DLG_CAPABILITY),
				stuInfo.dwMaxLogItems,
				ConvertString("MaxPageLogItem", DLG_CAPABILITY),
				stuInfo.dwMaxPageItems,
				ConvertString("IsSupportStartNo", DLG_CAPABILITY),
				stuInfo.bSupportStartNo ? ConvertString("Yes", DLG_CAPABILITY) : ConvertString("No", DLG_CAPABILITY),
				ConvertString("IsSupportTypeFilter", DLG_CAPABILITY),
				stuInfo.bSupportTypeFilter ? ConvertString("Yes", DLG_CAPABILITY) : ConvertString("No", DLG_CAPABILITY),
				ConvertString("IsSupportTimeFilter", DLG_CAPABILITY),
				stuInfo.bSupportTimeFilter ? ConvertString("Yes", DLG_CAPABILITY) : ConvertString("No", DLG_CAPABILITY));
			csCap += csLogServiceCap;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}

BOOL CDlgCapability::GetAlarmInCap(CString& csCap)
{
	char szAlarmBuff[1024 * 3] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_ALARM, 0, szAlarmBuff, sizeof(szAlarmBuff), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_ALARM_INFO stuAlarmInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_ALARM, szAlarmBuff, &stuAlarmInfo, sizeof(CFG_CAP_ALARM_INFO), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_ALARM_INFO))
		{		
			CString csAlarmCap;
			
			csAlarmCap.Format("%s = %d, \r\n\
%s = %d, \r\n\
%s = %d %s, \r\n\
%s = %d, \r\n\
%s = %d, \r\n%s = %d\r\n",
				ConvertString("AlarmPir", DLG_CAPABILITY),
				stuAlarmInfo.bAlarmPir, 
				ConvertString("Flash", DLG_CAPABILITY),
				stuAlarmInfo.bFlashLight,
				ConvertString("MaxDelay", DLG_CAPABILITY),
				stuAlarmInfo.nMaxDelay, 
				ConvertString("sec", DLG_CAPABILITY),
				ConvertString("AlarmBellCount", DLG_CAPABILITY),
				stuAlarmInfo.nAlarmBellCount, 
				ConvertString("MaxBackupAlarmServer", DLG_CAPABILITY),
				stuAlarmInfo.nMaxBackupAlarmServer, 
				ConvertString("MaxPSTNAlarmServer", DLG_CAPABILITY),
				stuAlarmInfo.nMaxPSTNAlarmServer);
			csCap += csAlarmCap;
			for (int n = 0; n < stuAlarmInfo.nDefenceAreaTypeNum; n++)
			{
				CString csAreaType;
				csAreaType.Format("%s %d: ", ConvertString("AreaType", DLG_CAPABILITY), n);
				switch (stuAlarmInfo.emDefenceAreaType[n])
				{
				case EM_CFG_DefenceAreaType_InTime:
					//csAreaType += "\"InTime\"\r\n";
					csAreaType += ConvertString("InTime", DLG_CAPABILITY);
					break;
				case EM_CFG_DefenceAreaType_Delay:
					//csAreaType += "\"Delay\"\r\n";
					csAreaType += ConvertString("Delay", DLG_CAPABILITY);
					break;
				case EM_CFG_DefenceAreaType_FullDay:
					//csAreaType += "\"FullDay\"\r\n";
					csAreaType += ConvertString("FullDay", DLG_CAPABILITY);
					break;
				case EM_CFG_DefenceAreaType_Unknown:
					//csAreaType += "\"Unknown\"\r\n";
					csAreaType += ConvertString("Unknown", DLG_CAPABILITY);
					break;
				}
				
				csCap += csAreaType;
				csCap += "\r\n";
			}
			for (int i = 0; i < stuAlarmInfo.nAlarmChannelNum; ++i)
			{
				static int s_nCount = 0;
				CString csAlarmChannel;
				csAlarmChannel.Format("%s %d %s:", 
					ConvertString("AlarmChannel", DLG_CAPABILITY), 
					i + 1, 
					ConvertString("support sense method", DLG_CAPABILITY));
				for (int j = 0; j < stuAlarmInfo.stuAlarmChannel[i].nSupportSenseMethodNum; ++j)
				{
					if (s_nCount >= 3)
					{
						csAlarmChannel += "\r\n";
						s_nCount = 0;
					}

					switch (stuAlarmInfo.stuAlarmChannel[i].emSupportSenseMethod[j])
					{
					case EM_SENSE_DOOR:
						csAlarmChannel += ConvertString("DoorMagnetism", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_PASSIVEINFRA:
						csAlarmChannel += ConvertString("PassiveInfrared", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_GAS:
						csAlarmChannel += ConvertString("GasSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_SMOKING:
						csAlarmChannel += ConvertString("SmokingSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_WATER:
						csAlarmChannel += ConvertString("WaterSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_ACTIVEFRA:
						csAlarmChannel += ConvertString("ActiveInfrared", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_GLASS:
						csAlarmChannel += ConvertString("GlassSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_EMERGENCYSWITCH:
						csAlarmChannel += ConvertString("EmergencySwitch", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_SHOCK:
						csAlarmChannel += ConvertString("ShockSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_DOUBLEMETHOD:
						csAlarmChannel += ConvertString("DoubleMethod", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_THREEMETHOD:
						csAlarmChannel += ConvertString("ThreeMethod", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_TEMP:
						csAlarmChannel += ConvertString("TempSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					case EM_SENSE_HUMIDITY:
						csAlarmChannel += ConvertString("HumiditySensor", DLG_CFG_ALARM);s_nCount++;
						break;
                    case EM_SENSE_WIND:
                        csAlarmChannel += ConvertString("WindSensor", DLG_CFG_ALARM);s_nCount++;
                        break;
					case EM_SENSE_CALLBUTTON:
						csAlarmChannel += ConvertString("CallButton", DLG_CFG_ALARM);s_nCount++;
						break;
					default :
						csAlarmChannel += ConvertString("OtherSensor", DLG_CFG_ALARM);s_nCount++;
						break;
					}
				}

				s_nCount = 0;
				csCap += csAlarmChannel;
				csCap += "\r\n";
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetEventHandlerCap(CString& csCap)
{
	char szBuf[1024] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_EVENTMANAGER, 0, szBuf, sizeof(szBuf), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_EVENTMANAGER_INFO stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_EVENTMANAGER, szBuf, &stuInfo, sizeof(stuInfo), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_EVENTMANAGER_INFO))
		{
			CString csEventManager;
			csEventManager.Format("%s : \r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s,\r\n\
%s = %s\r\n", 
				ConvertString("EventHandler Ability", DLG_CAPABILITY),
				ConvertString("TimeSectionEnable", DLG_CAPABILITY),
				stuInfo.bTimeSectionEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("RecordEnable", DLG_CAPABILITY),
				stuInfo.bRecordEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("AlarmOutEnable", DLG_CAPABILITY),
				stuInfo.bAlarmOutEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("PtzLinkEnable", DLG_CAPABILITY),
				stuInfo.bPtzLinkEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY),
				ConvertString("SnapshotEnable", DLG_CAPABILITY),
				stuInfo.bSnapshotEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("MailEnable", DLG_CAPABILITY),
				stuInfo.bMailEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("BeepEnable", DLG_CAPABILITY),
				stuInfo.bBeepEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("DejitterEnable", DLG_CAPABILITY),
				stuInfo.bDejitterEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY),
				ConvertString("TipEnable", DLG_CAPABILITY),
				stuInfo.bTipEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("MonitorTourEnable", DLG_CAPABILITY),
				stuInfo.bMonitorTourEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("MMSEnable", DLG_CAPABILITY),
				stuInfo.bMMSEnable ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("SupportAlarmBell", DLG_CAPABILITY),
				stuInfo.bSupportAlarmBell ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("SupportAlarmServer", DLG_CAPABILITY),
				stuInfo.bSupportAlarmServer ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY),
				ConvertString("SupportPtzLinkDelay", DLG_CAPABILITY),
				stuInfo.bSupportPtzLinkDelay ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY), 
				ConvertString("SupportPSTNAlarmServer", DLG_CAPABILITY),
				stuInfo.bSupportPSTNAlarmServer ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY),
				ConvertString("SupportAccessControl", DLG_CAPABILITY),
				stuInfo.bSupportAccessControl ? ConvertString("Enable", DLG_CAPABILITY) : ConvertString("Disable", DLG_CAPABILITY));
			csCap += csEventManager;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetExAlarmBoxCap(CString& csCap)
{
	for (int i = 0; i < MAX_EXALARMBOX_NUM; ++i)
	{
		char szBuf[1024] = {0};
		int nError = 0;
		BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_EXALARMBOX, i, szBuf, sizeof(szBuf), &nError, SDK_API_WAIT);
		if (bRet)
		{
			CFG_CAP_EXALARMBOX_INFO stuInfo = {0};
			DWORD dwRet = 0;
			bRet = CLIENT_ParseData(CFG_CAP_CMD_EXALARMBOX, szBuf, &stuInfo, sizeof(CFG_CAP_EXALARMBOX_INFO), &dwRet);
			if (bRet && dwRet == sizeof(CFG_CAP_EXALARMBOX_INFO))
			{
				CString csExAlarmBox;
				csExAlarmBox.Format("%s :\r\n %s : %d, %s = %d, %s = %d \r\n", 
					ConvertString("ExAlarmBox ability", DLG_CAPABILITY), ConvertString("ExAlarmBox channel", DLG_CAPABILITY), i, ConvertString("ExAlarmInCount", DLG_CAPABILITY),
					stuInfo.nAlarmInCount, ConvertString("ExAlarmOutCount", DLG_CAPABILITY), stuInfo.nAlarmOutCount);
				csCap += csExAlarmBox;
				csCap += "\r\n";
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CDlgCapability::GetExAlarmInCap(CString& csCap)
{
	char szBuf[1024 * 3] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_EXALARM, -1, szBuf, sizeof(szBuf), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CAP_EXALARM_INFO stuExAlarmInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_EXALARM, szBuf, &stuExAlarmInfo, sizeof(stuExAlarmInfo), &dwRet);
		if (bRet && dwRet == sizeof(CAP_EXALARM_INFO))
		{		
			CString csExAlarmCap;
			
			csExAlarmCap.Format("%s = %d, %s = %d\r\n ", ConvertString("ExAlarmIn", DLG_CAPABILITY),
				stuExAlarmInfo.nAlarmInCount, ConvertString("ExAlarmOut", DLG_CAPABILITY), stuExAlarmInfo.nAlarmOutCount);
			csCap += csExAlarmCap;
			for (int i = 0; i < stuExAlarmInfo.nExAlarmChannelNum; ++i)
			{
				CString csExAlarmChannel;
				csExAlarmChannel.Format("%s %d %s", ConvertString("ExAlarmChannel", DLG_CAPABILITY), i, ConvertString("support sense method:", DLG_CAPABILITY));
				for (int j = 0; j < stuExAlarmInfo.stuExAlarmChannel[i].nSupportSenseMethodNum; ++j)
				{
					switch (stuExAlarmInfo.stuExAlarmChannel[i].emSupportSenseMethod[j])
					{
					case EM_SENSE_DOOR:
						csExAlarmChannel += ConvertString("DoorMagnetism", DLG_CFG_ALARM);
						break;
					case EM_SENSE_PASSIVEINFRA:
						csExAlarmChannel += ConvertString("PassiveInfrared", DLG_CFG_ALARM);
						break;
					case EM_SENSE_GAS:
						csExAlarmChannel += ConvertString("GasSensor", DLG_CFG_ALARM);
						break;
					case EM_SENSE_SMOKING:
						csExAlarmChannel += ConvertString("SmokingSensor", DLG_CFG_ALARM);
						break;
					case EM_SENSE_WATER:
						csExAlarmChannel += ConvertString("WaterSensor", DLG_CFG_ALARM);
						break;
					case EM_SENSE_ACTIVEFRA:
						csExAlarmChannel += ConvertString("ActiveInfrared", DLG_CFG_ALARM);
						break;
					case EM_SENSE_GLASS:
						csExAlarmChannel += ConvertString("GlassSensor", DLG_CFG_ALARM);
						break;
					case EM_SENSE_EMERGENCYSWITCH:
						csExAlarmChannel += ConvertString("EmergencySwitch", DLG_CFG_ALARM);
						break;
					case EM_SENSE_SHOCK:
						csExAlarmChannel += ConvertString("ShockSensor", DLG_CFG_ALARM);
						break;
					case EM_SENSE_DOUBLEMETHOD:
						csExAlarmChannel += ConvertString("DoubleMethod", DLG_CFG_ALARM);
						break;
					case EM_SENSE_THREEMETHOD:
						csExAlarmChannel += ConvertString("ThreeMethod", DLG_CFG_ALARM);
						break;
					case EM_SENSE_TEMP:
						csExAlarmChannel += ConvertString("TempSensor", DLG_CFG_ALARM);
						break;
					case EM_SENSE_HUMIDITY:
						csExAlarmChannel += ConvertString("HumiditySensor", DLG_CFG_ALARM);
						break;
                    case EM_SENSE_WIND:
                        csExAlarmChannel += ConvertString("WindSensor", DLG_CFG_ALARM);
                        break;
					case EM_SENSE_CALLBUTTON:
						csExAlarmChannel += ConvertString("CallButton", DLG_CFG_ALARM);
						break;
					default :
						csExAlarmChannel += ConvertString("OtherSensor", DLG_CFG_ALARM);
						break;
					}
				}
				csCap += csExAlarmChannel;
				csCap += "\r\n";
			}
			int k = 0;
			CString csExAlarmProtocol;
			if (stuExAlarmInfo.nProtocolNum > 0)
			{
				csExAlarmProtocol.Format("support Protocol is : ");
			}
			for (k = 0; k < stuExAlarmInfo.nProtocolNum; ++k)
			{
				switch (stuExAlarmInfo.emProtocolType[k])
				{
				case CFG_EM_EXALARM_PROTOCOL_DH_ALARMBOX:
					csExAlarmProtocol += ConvertString("DH_AlarmBox", DLG_CAPABILITY);
					break;
				default :
					csExAlarmProtocol += ConvertString("UnKnow", DLG_CAPABILITY);
					break;
				}
			}
			csCap += csExAlarmProtocol;
			csCap += "\r\n";

			CString csExAlarmDefenceAreaType;
			if (stuExAlarmInfo.nDefenceAreaTypeNum > 0)
			{
				csExAlarmDefenceAreaType.Format("%s ", ConvertString("support DefenceAreaType is :", DLG_CAPABILITY));
			}
			for (k = 0; k < stuExAlarmInfo.nDefenceAreaTypeNum; ++k)
			{
				switch (stuExAlarmInfo.emDefenceAreaType[k])
				{
				case EM_CFG_DefenceAreaType_InTime:
					csExAlarmDefenceAreaType += ConvertString("Intime", DLG_CAPABILITY);
					break;
				case EM_CFG_DefenceAreaType_Delay:
					csExAlarmDefenceAreaType += ConvertString("Delay", DLG_CAPABILITY);
					break;
				case EM_CFG_DefenceAreaType_FullDay:
					csExAlarmDefenceAreaType += ConvertString("Fullday", DLG_CAPABILITY);
					break;
				default :
					csExAlarmDefenceAreaType += ConvertString("UnKnow", DLG_CAPABILITY);
					break;
				}
			}
			csCap += csExAlarmDefenceAreaType;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetAccessControlCap(CString& csCap)
{
	char szBuf[1024] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_ACCESSCONTROLMANAGER, -1, szBuf, sizeof(szBuf), &nError, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_ACCESSCONTROL stuCap = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_ACCESSCONTROLMANAGER, szBuf, &stuCap, sizeof(stuCap), &dwRet);
		if (bRet && dwRet == sizeof(CFG_CAP_ACCESSCONTROL))
		{			
			csCap += ConvertString("AccessControlManager Cap:", DLG_CAPABILITY);

			csCap += "\r\n";
			CString csAccessControl;
			csAccessControl.Format("%s = %d \r\n", 
				ConvertString("support access count ", DLG_CAPABILITY),
				stuCap.nAccessControlGroups);
			csCap += csAccessControl;
			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetAnalogAlarmCap(CString& csCap)
{
	char szBuf[1024 * 10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_ANALOGALARM, -1, szBuf, sizeof(szBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_ANALOGALARM stuInfo = {0};
		int nRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_ANALOGALARM, szBuf, &stuInfo, sizeof(stuInfo), &nRet);
		if (bRet && nRet == sizeof(CFG_CAP_ANALOGALARM))
		{
			csCap += ConvertString("AnalogAlarm Cap:", DLG_CAPABILITY);
			csCap += "\r\n";

			CString csTmp;
			csTmp.Format("%s = %d\r\n", ConvertString("Local", DLG_CAPABILITY), stuInfo.dwLocalNum);
			csCap += csTmp;

			csTmp.Format("%s = %d\r\n", ConvertString("ExLocal", DLG_CAPABILITY), stuInfo.dwExLocalNum);
			csCap += csTmp;

			csCap += "\r\n";
		}
		else
		{
			return FALSE;
		}
	} 
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCapability::GetLowRateWPanCap(CString& csCap)
{
    char szBuf[1024 * 10] = {0};
    int nErr = 0;
    BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogin, CFG_CAP_CMD_LOWRATEWPAN, -1, szBuf, sizeof(szBuf), &nErr, SDK_API_WAIT);
    if (bRet)
    {
        CFG_CAP_LOWRATEWPAN stuInfo = {0};
        int nRet = 0;
        bRet = CLIENT_ParseData(CFG_CAP_CMD_LOWRATEWPAN, szBuf, &stuInfo, sizeof(stuInfo), &nRet);
        if (bRet && nRet == sizeof(CFG_CAP_LOWRATEWPAN))
        {
            csCap += ConvertString("LowRateWPan Cap:", DLG_CAPABILITY);
            csCap += "\r\n";
            
            CString csTmp;
            csTmp.Format("%s = %d\r\n", ConvertString("MaxPageSize", DLG_CAPABILITY), stuInfo.nMaxPageSize);
            csCap += csTmp;
            
            csCap += "\r\n";
        }
        else
        {
            return FALSE;
        }
    } 
    else
    {
        return FALSE;
	}
    return TRUE;
}