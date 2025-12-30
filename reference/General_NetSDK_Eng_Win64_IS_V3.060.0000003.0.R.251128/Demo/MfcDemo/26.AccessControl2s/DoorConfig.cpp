// DoorConfig.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "DoorConfig.h"
#include "DlgCfgDoorOpenTimeSection.h"
#include "TimeSchedule.h"
#include "Holiday.h"

IMPLEMENT_DYNAMIC(DoorConfig, CDialog)

DoorConfig::DoorConfig(CWnd* pParent /*=NULL*/)
	: CDialog(DoorConfig::IDD, pParent)
{
	memset(&m_stuInfo,0,sizeof(m_stuInfo));
}

DoorConfig::~DoorConfig()
{
}

void DoorConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOORCFG_CHK_FIRSTENTER_REMOTECHECK, m_chkRemoteCheck);
	DDX_Control(pDX, IDC_DOORCFG_CHK_FIRSTENTER_ENABLE, m_chkFirstEnterEnable);
	DDX_Control(pDX, IDC_DOORCFG_CMB_FIRSTENTER_STATUS, m_cbFirstEnterStatus);
	DDX_Control(pDX, IDC_DOORCFG_CMB_CHANNEL, m_cmbChannel);
	DDX_Control(pDX, IDC_DOORCFG_CMB_OPENTIMEINDEX, m_cmbOpenTimeIndex);
	DDX_Control(pDX, IDC_DOORCFG_CMB_OPENMETHOD, m_cmbOpenMethod);
	DDX_Control(pDX, IDC_DOORCFG_CHK_SENSOR, m_chkSensor);
	DDX_Control(pDX, IDC_DOORCFG_CHK_REPEATENTERALARM, m_chkRepeatEnterAlarm);
	DDX_Control(pDX, IDC_DOORCFG_CHK_DURESSALARM, m_chkDuressAlarm);
	DDX_Control(pDX, IDC_DOORCFG_CHK_DOORNOTCLOSEALARM, m_chkDoorNotCloseAlarm);
	DDX_Control(pDX, IDC_DOORCFG_CHK_BREAKINALARM, m_chkBreakAlarm);
	DDX_Control(pDX, IDC_DOORCFG_CMB_ACCESSSTATE, m_cmbAccessState);
	DDX_Control(pDX, IDC_DOORCFG_CHECK_AUTOREMOTE, m_chkAutoRemoteTimeEnable);
	DDX_Control(pDX, IDC_DOORCFG_CHK_MALICIOUSALARM, m_chkMaliciousAlarm);
	DDX_Control(pDX, IDC_DOORCFG_CHK_CUSTOMPASSWORD, m_chkCustomPassword);
}

BEGIN_MESSAGE_MAP(DoorConfig, CDialog)
	ON_BN_CLICKED(IDC_DOORCFG_BTN_GET, &DoorConfig::OnBnClickedDoorcfgBtnGet)
	ON_BN_CLICKED(IDC_DOORCFG_BTN_SET, &DoorConfig::OnBnClickedDoorcfgBtnSet)
	ON_BN_CLICKED(IDC_DOORCFG_BTN_TIMESECTION, &DoorConfig::OnBnClickedDoorcfgBtnTimesection)
	ON_BN_CLICKED(IDC_BUT_TIMESCHEDULECONFIG, &DoorConfig::OnBnClickedButTimescheduleconfig)
	ON_BN_CLICKED(IDC_BUT_HOLIDAYMANAGE, &DoorConfig::OnBnClickedButHolidaymanage)
	ON_CBN_SELCHANGE(IDC_DOORCFG_CMB_CHANNEL, &DoorConfig::OnCbnSelchangeDoorcfgCmbChannel)
END_MESSAGE_MAP()


BOOL DoorConfig::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_DOORCFG_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOORCFG_BTN_SET)->EnableWindow(FALSE);
	}
	else
	{	
		OnBnClickedDoorcfgBtnGet();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void DoorConfig::InitDlg()
{
	int i = 0;

	// channel
	m_cmbChannel.ResetContent();
	int nChn = 4;
	int nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(nAccessGroup);
	if (bret && nAccessGroup > 0)
	{
		nChn = nAccessGroup;
	}
	for (i = 0; i < nChn; i++)
	{
		char szContent[8] = {0};
		_snprintf(szContent, sizeof(szContent), "%d", i + 1);
		m_cmbChannel.AddString(szContent);
		m_cmbChannel.SetItemData(i, (DWORD)i);
	}
	m_cmbChannel.SetCurSel(0);	

	// door open method
	m_cmbOpenMethod.ResetContent();
	for (i = 0; i < sizeof(stuDemoOpenMethod)/sizeof(stuDemoOpenMethod[0]); i++)
	{
		m_cmbOpenMethod.InsertString(-1, ConvertString(stuDemoOpenMethod[i].szName));
	}
	m_cmbOpenMethod.SetCurSel(-1);
	m_cmbOpenMethod.SetDroppedWidth(160);

	//access state
	m_cmbAccessState.ResetContent();
	for (i = 0; i < sizeof(stuDemoAccessState)/sizeof(stuDemoAccessState[0]); i++)
	{
		m_cmbAccessState.InsertString(-1, ConvertString(stuDemoAccessState[i].szName));
	}
	m_cmbAccessState.SetCurSel(-1);
	m_cmbAccessState.SetDroppedWidth(160);

	m_stuInfo.abDoorOpenMethod = true;

	// door unlock hold time
	m_stuInfo.abUnlockHoldInterval = true;

	// door close timeout
	m_stuInfo.abCloseTimeout = true;

	// open time index in config of AccessTimeSchedule, start from 0
	m_cmbOpenTimeIndex.ResetContent();
	for (i = 0; i < 128; i++)
	{
		CString csInfo;
		csInfo.Format("%02d", i);
		m_cmbOpenTimeIndex.InsertString(-1, csInfo);
	}

	// first enter status
	m_cbFirstEnterStatus.ResetContent();
	static const char* szFirstEnterStat[] = 
	{	"Unknown", 
	"KeepOpen", 
	"Normal"
	};
	for (i=0; i<3; i++)
	{
		m_cbFirstEnterStatus.InsertString(-1, ConvertString(szFirstEnterStat[i]));
	}
	m_cbFirstEnterStatus.SetCurSel((int)m_stuInfo.stuFirstEnterInfo.emStatus);

	SetDlgItemInt(IDC_DOORCFG_EDT_FIRSTENTER_TIMERINDEX, 0);

	m_cmbOpenTimeIndex.SetCurSel(-1);
	m_stuInfo.abOpenAlwaysTimeIndex = true;

	// holiday record set recNo
	m_stuInfo.abHolidayTimeIndex = true;

	// break in alarm enable
	m_chkBreakAlarm.SetCheck(BST_UNCHECKED);

	// repeat enter alarm enable
	m_chkRepeatEnterAlarm.SetCheck(BST_UNCHECKED);

	// door not close enable
	m_chkDoorNotCloseAlarm.SetCheck(BST_UNCHECKED);

	// duress alarm enable
	m_chkDuressAlarm.SetCheck(BST_UNCHECKED);

	// sensor alarm enable
	m_chkSensor.SetCheck(BST_UNCHECKED);

	// first enter
	m_chkFirstEnterEnable.SetCheck(BST_UNCHECKED);

	// remote check
	m_chkRemoteCheck.SetCheck(BST_UNCHECKED);
}

void DoorConfig::OnBnClickedDoorcfgBtnGet()
{
	int nChn = m_cmbChannel.GetCurSel();
	if (-1 == nChn)
	{
		return;
	}
	BOOL bret = Device::GetInstance().GetAccessEventCfg(nChn, m_stuInfo);
	if (bret)
	{
		SetDoorCfgInfoToCtrl();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void DoorConfig::OnBnClickedDoorcfgBtnSet()
{
	int nChn = m_cmbChannel.GetCurSel();
	if (-1 == nChn)
	{
		return;
	}
	GetDoorCfgInfoFromCtrl();
	BOOL bret = Device::GetInstance().SetAccessEventCfg(nChn, &m_stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void DoorConfig::GetDoorCfgInfoFromCtrl()
{
	// door open method
	int nMethodIndex = m_cmbOpenMethod.GetCurSel();
	if (nMethodIndex >= 0 && nMethodIndex < sizeof(stuDemoOpenMethod)/sizeof(stuDemoOpenMethod[0]))
	{
		m_stuInfo.emDoorOpenMethod = (CFG_DOOR_OPEN_METHOD)stuDemoOpenMethod[nMethodIndex].emOpenMethod;
	}
	else
	{
		m_stuInfo.emDoorOpenMethod = CFG_DOOR_OPEN_METHOD_UNKNOWN;
	}

	// access state
	int nAccessIndex = m_cmbAccessState.GetCurSel();
	if (nAccessIndex >= 0 && nAccessIndex < sizeof(stuDemoAccessState)/sizeof(stuDemoAccessState[0]))
	{
		m_stuInfo.emState = (CFG_ACCESS_STATE)stuDemoAccessState[nAccessIndex].emAccessStart;
	}
	else
	{
		m_stuInfo.emState = ACCESS_STATE_NORMAL;
	}

	// door unlock hold time
	m_stuInfo.nUnlockHoldInterval = GetDlgItemInt(IDC_DOORCFG_EDT_UNLOCKHOLD, NULL, TRUE);
	if (m_stuInfo.nUnlockHoldInterval > 0)
	{
		m_stuInfo.abUnlockHoldInterval = true;
	}

	// door close timeout
	m_stuInfo.nCloseTimeout = GetDlgItemInt(IDC_DOORCFG_EDT_CLOSETIMEOUT, NULL, TRUE);
	if (m_stuInfo.nCloseTimeout > 0)
	{
		m_stuInfo.abCloseTimeout = true;
	}

	// open time index in config of AccessTimeSchedule, start from 0
	if (m_cmbOpenTimeIndex.GetCurSel() == -1)
	{
		m_stuInfo.nOpenAlwaysTimeIndex = 255;
	}
	else
	{
		m_stuInfo.nOpenAlwaysTimeIndex = m_cmbOpenTimeIndex.GetCurSel();
	}

	// holiday record set recNo
	m_stuInfo.nHolidayTimeRecoNo = GetDlgItemInt(IDC_DOORCFG_EDT_HOLIDAYTIMERECNO, NULL, TRUE);

	// first enter status
	m_stuInfo.stuFirstEnterInfo.emStatus = static_cast<CFG_ACCESS_FIRSTENTER_STATUS>(m_cbFirstEnterStatus.GetCurSel());

	// first enter time index
	m_stuInfo.stuFirstEnterInfo.nTimeIndex = GetDlgItemInt(IDC_DOORCFG_EDT_FIRSTENTER_TIMERINDEX);

	// break in alarm enable
	if (m_chkBreakAlarm.GetCheck())
	{
		m_stuInfo.abBreakInAlarmEnable = true;
		m_stuInfo.bBreakInAlarmEnable = TRUE;
	}
	else
	{
		m_stuInfo.bBreakInAlarmEnable = FALSE;
	}

	// repeat enter alarm enable
	if (m_chkRepeatEnterAlarm.GetCheck())
	{
		m_stuInfo.abRepeatEnterAlarmEnable = true;
		m_stuInfo.bRepeatEnterAlarm = TRUE;
	}
	else
	{
		m_stuInfo.bRepeatEnterAlarm = FALSE;
	}

	// door not close enable
	if (m_chkDoorNotCloseAlarm.GetCheck())
	{
		m_stuInfo.abDoorNotClosedAlarmEnable = true;
		m_stuInfo.bDoorNotClosedAlarmEnable = TRUE;
	}
	else
	{
		m_stuInfo.bDoorNotClosedAlarmEnable = FALSE;
	}

	// duress alarm enable
	if (m_chkDuressAlarm.GetCheck())
	{
		m_stuInfo.abDuressAlarmEnable = true;
		m_stuInfo.bDuressAlarmEnable = TRUE;
	}
	else
	{
		m_stuInfo.bDuressAlarmEnable = FALSE;
	}

	// sensor alarm enable
	if (m_chkSensor.GetCheck())
	{
		m_stuInfo.abSensorEnable = true;
		m_stuInfo.bSensorEnable = TRUE;
	}
	else
	{
		m_stuInfo.bSensorEnable = FALSE;
	}

	// time section...

	// first enter enable
	if (m_chkFirstEnterEnable.GetCheck())
	{
		m_stuInfo.abFirstEnterEnable = true;
		m_stuInfo.stuFirstEnterInfo.bEnable = TRUE;
	}
	else
	{
		m_stuInfo.stuFirstEnterInfo.bEnable = FALSE;
	}

	// remote check
	if (m_chkRemoteCheck.GetCheck())
	{
		m_stuInfo.abRemoteCheck = true;
		m_stuInfo.bRemoteCheck = TRUE;
	}
	else
	{
		m_stuInfo.bRemoteCheck = FALSE;
	}

	// m_chkMaliciousAlarm check
	if (m_chkMaliciousAlarm.GetCheck())
	{
		m_stuInfo.bUnAuthorizedMaliciousSwipEnable = TRUE;
	}
	else
	{
		m_stuInfo.bUnAuthorizedMaliciousSwipEnable = FALSE;
	}

	if (m_chkCustomPassword.GetCheck())
	{
		m_stuInfo.bCustomPasswordEnable = TRUE;
	}
	else
	{
		m_stuInfo.bCustomPasswordEnable = FALSE;
	}

	CString strTemp;
	GetDlgItem(IDC_DOORCFG_EDT_OPENALWAYS)->GetWindowText(strTemp);
	m_stuInfo.nOpenAlwaysTimeIndex = atoi(strTemp);
	m_stuInfo.abOpenAlwaysTimeIndex = true/*m_chkOpenAlways.GetCheck()*/;

	GetDlgItem(IDC_DOORCFG_EDT_CLOSEALWAYS)->GetWindowText(strTemp);
	m_stuInfo.nCloseAlwaysTimeIndex = atoi(strTemp);
	m_stuInfo.abCloseAlwaysTimeIndex = true/*m_chkCloseAlways.GetCheck()*/;

	GetDlgItem(IDC_DOORCFG_EDT_AUTOREMOTE)->GetWindowText(strTemp);
	m_stuInfo.stuAutoRemoteCheck.nTimeSechdule = atoi(strTemp);
	m_stuInfo.stuAutoRemoteCheck.bEnable = m_chkAutoRemoteTimeEnable.GetCheck();
}

void DoorConfig::SetDoorCfgInfoToCtrl()
{
	// door open method
	BOOL bMethodFound = FALSE;
	for (int i = 0; i < sizeof(stuDemoOpenMethod)/sizeof(stuDemoOpenMethod[0]); i++)
	{
		if (m_stuInfo.emDoorOpenMethod == stuDemoOpenMethod[i].emOpenMethod)
		{
			bMethodFound = TRUE;
			m_cmbOpenMethod.SetCurSel(i);
			break;
		}
	}
	if (!bMethodFound)
	{
		m_cmbOpenMethod.SetCurSel(-1);
	}

	// access state
	BOOL bStateFound = FALSE;
	for (int i = 0; i < sizeof(stuDemoAccessState)/sizeof(stuDemoAccessState[0]); i++)
	{
		if (m_stuInfo.emState == stuDemoAccessState[i].emAccessStart)
		{
			bStateFound = TRUE;
			m_cmbAccessState.SetCurSel(i);
			break;
		}
	}
	if (!bStateFound)
	{
		m_cmbAccessState.SetCurSel(-1);
	}

	// door unlock hold time
	SetDlgItemInt(IDC_DOORCFG_EDT_UNLOCKHOLD, m_stuInfo.nUnlockHoldInterval);

	// door close timeout
	SetDlgItemInt(IDC_DOORCFG_EDT_CLOSETIMEOUT, m_stuInfo.nCloseTimeout);

	// open time index in config of AccessTimeSchedule, start from 0
	m_cmbOpenTimeIndex.SetCurSel(m_stuInfo.nOpenAlwaysTimeIndex);

	// holiday record set recNo
	SetDlgItemInt(IDC_DOORCFG_EDT_HOLIDAYTIMERECNO, m_stuInfo.nHolidayTimeRecoNo);

	// first enter time index
	SetDlgItemInt(IDC_DOORCFG_EDT_FIRSTENTER_TIMERINDEX, m_stuInfo.stuFirstEnterInfo.nTimeIndex);

	// first enter status
	m_cbFirstEnterStatus.SetCurSel((int)m_stuInfo.stuFirstEnterInfo.emStatus);


	// break in alarm enable
	if (m_stuInfo.abBreakInAlarmEnable)
	{
		if (m_stuInfo.bBreakInAlarmEnable)
		{
			m_chkBreakAlarm.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkBreakAlarm.SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		m_chkBreakAlarm.SetCheck(BST_UNCHECKED);
	}

	// repeat enter alarm enable
	if (m_stuInfo.abRepeatEnterAlarmEnable)
	{
		if (m_stuInfo.bRepeatEnterAlarm)
		{
			m_chkRepeatEnterAlarm.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkRepeatEnterAlarm.SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		m_chkRepeatEnterAlarm.SetCheck(BST_UNCHECKED);
	}

	// door not close enable
	if (m_stuInfo.abDoorNotClosedAlarmEnable)
	{
		if (m_stuInfo.bDoorNotClosedAlarmEnable)
		{
			m_chkDoorNotCloseAlarm.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkDoorNotCloseAlarm.SetCheck(BST_UNCHECKED);
		}
	} 
	else
	{
		m_chkDoorNotCloseAlarm.SetCheck(BST_UNCHECKED);
	}

	// duress alarm enable
	if (m_stuInfo.abDuressAlarmEnable)
	{
		if (m_stuInfo.bDuressAlarmEnable)
		{
			m_chkDuressAlarm.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkDuressAlarm.SetCheck(BST_UNCHECKED);
		}
	} 
	else
	{
		m_chkDuressAlarm.SetCheck(BST_UNCHECKED);
	}

	// sensor alarm enable
	if (m_stuInfo.abSensorEnable)
	{
		if (m_stuInfo.bSensorEnable)
		{
			m_chkSensor.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkSensor.SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		m_chkSensor.SetCheck(BST_UNCHECKED);
	}

	// time section...

	//first enter enable
	if (m_stuInfo.abFirstEnterEnable)
	{
		if (m_stuInfo.stuFirstEnterInfo.bEnable)
		{
			m_chkFirstEnterEnable.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkFirstEnterEnable.SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		m_chkFirstEnterEnable.SetCheck(BST_UNCHECKED);
	}

	// remote check
	if (m_stuInfo.abRemoteCheck)
	{
		if (m_stuInfo.bRemoteCheck)
		{
			m_chkRemoteCheck.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkRemoteCheck.SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		m_chkRemoteCheck.SetCheck(BST_UNCHECKED);
	}

	// m_chkMaliciousAlarm check
	if (m_stuInfo.bUnAuthorizedMaliciousSwipEnable)
	{
		m_chkMaliciousAlarm.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkMaliciousAlarm.SetCheck(BST_UNCHECKED);
	}

	if (m_stuInfo.bCustomPasswordEnable)
	{
		m_chkCustomPassword.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkCustomPassword.SetCheck(BST_UNCHECKED);
	}

	CString strTemp;
	strTemp.Format("%d",m_stuInfo.nOpenAlwaysTimeIndex);
	GetDlgItem(IDC_DOORCFG_EDT_OPENALWAYS)->SetWindowText(strTemp);


	strTemp.Format("%d",m_stuInfo.nCloseAlwaysTimeIndex);
	GetDlgItem(IDC_DOORCFG_EDT_CLOSEALWAYS)->SetWindowText(strTemp);


	strTemp.Format("%d",m_stuInfo.stuAutoRemoteCheck.nTimeSechdule);
	GetDlgItem(IDC_DOORCFG_EDT_AUTOREMOTE)->SetWindowText(strTemp);
	m_chkAutoRemoteTimeEnable.SetCheck(m_stuInfo.stuAutoRemoteCheck.bEnable);
}

void DoorConfig::OnBnClickedDoorcfgBtnTimesection()
{
	DlgCfgDoorOpenTimeSection dlg(this);
	dlg.SetTimeSection(&m_stuInfo.stuDoorTimeSection[0][0]);
	if (IDOK == dlg.DoModal())
	{
		for (int i = 0; i < WEEK_DAY_NUM; i++)
		{
			for (int j = 0; j < MAX_DOOR_TIME_SECTION; j++)
			{
				const CFG_DOOROPEN_TIMESECTION_INFO* pstuTimeSection = dlg.GetTimeSection(i, j);
				if (pstuTimeSection)
				{
					memcpy(&m_stuInfo.stuDoorTimeSection[i][j], pstuTimeSection, sizeof(CFG_DOOROPEN_TIMESECTION_INFO));
				}
			}
		}
	}

}

void DoorConfig::OnBnClickedButTimescheduleconfig()
{
	TimeSchedule dlg(this);
	dlg.DoModal();
}

void DoorConfig::OnBnClickedButHolidaymanage()
{
	Holiday dlg(this);
	dlg.DoModal();
}

void DoorConfig::OnCbnSelchangeDoorcfgCmbChannel()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedDoorcfgBtnGet();
}
