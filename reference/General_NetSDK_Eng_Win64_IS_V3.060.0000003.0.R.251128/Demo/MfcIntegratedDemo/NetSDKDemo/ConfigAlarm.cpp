// ConfigAlarm.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigAlarm.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarm dialog


CConfigAlarm::CConfigAlarm(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigAlarm)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDev = 0;
	memset(&m_almCFG, 0, sizeof(DHDEV_ALARM_SCHEDULE));
	m_bInited = FALSE;
}


void CConfigAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigAlarm)
	DDX_Control(pDX, IDC_TAB_ALARMBOARD, m_alarmBoard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigAlarm, CDialog)
	//{{AFX_MSG_MAP(CConfigAlarm)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_ALARMBOARD, OnSelchangeTabAlarmboard)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_UNDO_ALL, OnUndoAll)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarm message handlers


#define PRIVATE_ALARM_UPLOAD		0x00000001
#define PRIVATE_ALARM_RECORD		0x00000002
#define PRIVATE_ALARM_PTZ		0x00000004
#define PRIVATE_ALARM_MAIL		0x00000008
#define PRIVATE_ALARM_TOUR		0x00000010
#define PRIVATE_ALARM_TIP		0x00000020
#define PRIVATE_ALARM_OUT		0x00000040

// #define PRIVATE_MOTION_ROW		12
// #define PRIVATE_MOTION_COL		16	

void CConfigAlarm::OnSelchangeTabAlarmboard(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch(m_alarmBoard.GetCurSel())
	{
	case 0: //alarm in
		m_alarmInDlg.ShowWindow(SW_SHOW);
		m_blindDlg.ShowWindow(SW_HIDE);
		m_diskDlg.ShowWindow(SW_HIDE);
		m_motionDlg.ShowWindow(SW_HIDE);
		m_vlostDlg.ShowWindow(SW_HIDE);
		break;
	case 1: //motion
		m_motionDlg.ShowWindow(SW_SHOW);
		m_alarmInDlg.ShowWindow(SW_HIDE);
		m_blindDlg.ShowWindow(SW_HIDE);
		m_diskDlg.ShowWindow(SW_HIDE);
		m_vlostDlg.ShowWindow(SW_HIDE);
		break;
	case 2: //video lost
		m_vlostDlg.ShowWindow(SW_SHOW);
		m_alarmInDlg.ShowWindow(SW_HIDE);
		m_blindDlg.ShowWindow(SW_HIDE);
		m_diskDlg.ShowWindow(SW_HIDE);
		m_motionDlg.ShowWindow(SW_HIDE);
		break;
	case 3: //blind
		m_blindDlg.ShowWindow(SW_SHOW);
		m_alarmInDlg.ShowWindow(SW_HIDE);
		m_diskDlg.ShowWindow(SW_HIDE);
		m_motionDlg.ShowWindow(SW_HIDE);
		m_vlostDlg.ShowWindow(SW_HIDE);
		break;
	case 4: //disk
		m_diskDlg.ShowWindow(SW_SHOW);
		m_alarmInDlg.ShowWindow(SW_HIDE);
		m_blindDlg.ShowWindow(SW_HIDE);
		m_motionDlg.ShowWindow(SW_HIDE);
		m_vlostDlg.ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}

	*pResult = 0;
}


BOOL CConfigAlarm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	m_alarmBoard.InsertItem(0, ConvertString(MSG_ALARMCFG_ALARMIN));
	m_alarmBoard.InsertItem(1, ConvertString(MSG_ALARMCFG_MOTION));
	m_alarmBoard.InsertItem(2, ConvertString(MSG_ALARMCFG_VIDEOLOST));
	m_alarmBoard.InsertItem(3, ConvertString(MSG_ALARMCFG_BLINE));

	m_alarmInDlg.SetDevice(m_pDev);
	m_blindDlg.SetDevice(m_pDev);
	m_motionDlg.SetDevice(m_pDev);
	m_vlostDlg.SetDevice(m_pDev);

	m_alarmInDlg.Create(IDD_CONFIG_ALARM_ALARM, &m_alarmBoard);
	m_blindDlg.Create(IDD_CONFIG_ALARM_BLIND, &m_alarmBoard);
	m_diskDlg.Create(IDD_CONFIG_ALARM_DISK, &m_alarmBoard);
	m_motionDlg.Create(IDD_CONFIG_ALARM_MOTION, &m_alarmBoard);
	m_vlostDlg.Create(IDD_CONFIG_ALARM_VLOST, &m_alarmBoard);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigAlarm::GetConfig()
{
	if (!m_pDev || m_bInited)
	{
		return;
	}
	
	DWORD retlen = 0;
	BOOL bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_ALARMCFG, 0,
								&m_almCFG, sizeof(DHDEV_ALARM_SCHEDULE), &retlen, CONFIG_WAITTIME);
	if (!bRet || retlen != sizeof(DHDEV_ALARM_SCHEDULE))
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		m_bInited = TRUE;
	}
	
	m_alarmInDlg.SetAlarmInCFG(m_almCFG.struLocalAlmIn);
	m_blindDlg.SetBlindCFG(m_almCFG.struBlind);
	m_motionDlg.SetMotionCFG(m_almCFG.struMotion);
	m_vlostDlg.SetVlostCFG(m_almCFG.struVideoLost);

	m_alarmBoard.SetCurSel(0);
	m_alarmInDlg.ShowWindow(SW_SHOW);
	m_blindDlg.ShowWindow(SW_HIDE);
	m_diskDlg.ShowWindow(SW_HIDE);
	m_motionDlg.ShowWindow(SW_HIDE);
	m_vlostDlg.ShowWindow(SW_HIDE);
}

void CConfigAlarm::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

void CConfigAlarm::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	GetConfig();
}

void CConfigAlarm::OnUndoAll() 
{
	m_bInited = FALSE;

	m_alarmInDlg.UndoAll();
	m_blindDlg.UndoAll();
	m_motionDlg.UndoAll();
	m_vlostDlg.UndoAll();

	GetConfig();
}

void CConfigAlarm::OnApply() 
{
	m_alarmInDlg.GetAlarmInCFG(m_almCFG.struLocalAlmIn);
	m_blindDlg.GetBlindCFG(m_almCFG.struBlind);
	m_motionDlg.GetMotionCFG(m_almCFG.struMotion);
	m_vlostDlg.GetVlostCFG(m_almCFG.struVideoLost);

	BOOL bRet = CLIENT_SetDevConfig(m_pDev->LoginID, DH_DEV_ALARMCFG, 0,
								&m_almCFG, sizeof(DHDEV_ALARM_SCHEDULE), CONFIG_WAITTIME);
	if (!bRet)
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		MessageBox(ConvertString(MSG_CONFIG_SUCCESS), "OK");
	}
}