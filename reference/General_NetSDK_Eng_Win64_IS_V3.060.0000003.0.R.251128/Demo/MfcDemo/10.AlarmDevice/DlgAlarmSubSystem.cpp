// DlgAlarmSubSystem.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAlarmSubSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmSubSystem dialog


CDlgAlarmSubSystem::CDlgAlarmSubSystem(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgAlarmSubSystem::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAlarmSubSystem)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
}


void CDlgAlarmSubSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarmSubSystem)
	DDX_Control(pDX, IDC_ALARMSUBSYSTEM_CMB_CHANNEL, m_cmbChannelId);
	DDX_Control(pDX, IDC_ALARMSUBSYSTEM_CK_ACTIVE, m_ckActive);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAlarmSubSystem, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarmSubSystem)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEM_BTN_GET, OnAlarmsubsystemBtnGet)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEM_BTN_SET, OnAlarmsubsystemBtnSet)
	ON_CBN_SELCHANGE(IDC_ALARMSUBSYSTEM_CMB_CHANNEL, OnSelchangeAlarmsubsystemCmbChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmSubSystem message handlers

BOOL CDlgAlarmSubSystem::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARMSUBSYSTEM);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_ALARMSUBSYSTEM), ConvertString("Prompt"));
		return TRUE;
	}

	// 个数需要查看配置数组大小
	for (int i = 0; i < 8; i++)
	{
		CString csNum;
		csNum.Format("%s %d", ConvertString("Channel", DLG_ALARMSUBSYSTEM), i + 1);
		m_cmbChannelId.InsertString(-1, csNum);
	}

	m_cmbChannelId.SetCurSel(0);
	OnAlarmsubsystemBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAlarmSubSystem::OnAlarmsubsystemBtnGet() 
{
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_ALARMSUBSYSTEM), ConvertString("Prompt"));
		return ;
	}
	int nRetLen = 0;
	NET_GET_ALARM_SUBSYSTEM_ACTIVATE_STATUES stuParam = {sizeof(NET_GET_ALARM_SUBSYSTEM_ACTIVATE_STATUES)};
	stuParam.nChannelId = m_cmbChannelId.GetCurSel();
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_GET_ALARM_SUBSYSTEM_ACTIVATESTATUS, 
		(char*)&stuParam, sizeof(NET_GET_ALARM_SUBSYSTEM_ACTIVATE_STATUES), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryActiveState failed:", DLG_ALARMSUBSYSTEM), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	} 
	else
	{
		m_ckActive.SetCheck(stuParam.bActive ? BST_CHECKED : BST_UNCHECKED);
	}

}

void CDlgAlarmSubSystem::OnAlarmsubsystemBtnSet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_ALARMSUBSYSTEM), ConvertString("Prompt"));
		return;
	}

	NET_CTRL_ALARM_SUBSYSTEM_SETACTIVE stuParam = {sizeof(NET_CTRL_ALARM_SUBSYSTEM_SETACTIVE)};
	stuParam.bActive = m_ckActive.GetCheck();
	stuParam.nChannelId = m_cmbChannelId.GetCurSel();
	BOOL bRet = CLIENT_ControlDevice(m_lLoginId, DH_CTRL_ALARM_SUBSYSTEM_ACTIVE_SET, &stuParam, SDK_API_WAIT);
	CString csOut;
	if (bRet)
	{
		MessageBox(ConvertString("Set Active status OK!", DLG_ALARMSUBSYSTEM), ConvertString("Prompt"));	
	} 
	else
	{
		csOut.Format("%s%08x", ConvertString("Set Active status failed:", DLG_ALARMSUBSYSTEM), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void CDlgAlarmSubSystem::OnSelchangeAlarmsubsystemCmbChannel() 
{
	// TODO: Add your control notification handler code here
	OnAlarmsubsystemBtnGet();
}
