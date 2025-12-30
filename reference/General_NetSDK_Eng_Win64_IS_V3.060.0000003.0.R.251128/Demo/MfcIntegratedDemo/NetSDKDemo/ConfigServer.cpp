// ConfigServer.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigServer.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigServer dialog


CConfigServer::CConfigServer(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigServer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigServer)
	//}}AFX_DATA_INIT
	m_pDev = 0;
	memset(&m_sysCFG, 0, sizeof(DHDEV_SYSTEM_ATTR_CFG));
	m_bInited = FALSE;
}


void CConfigServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigServer)
	DDX_Control(pDX, IDC_COMBO_DATESPR, m_datesprsel);
	DDX_Control(pDX, IDC_COMBO_TIMEFMT, m_timefmtsel);
	DDX_Control(pDX, IDC_COMBO_DATEFMT, m_datefmtsel);
	DDX_Control(pDX, IDC_COMBO_VIDEOFMT, m_videofmtsel);
	DDX_Control(pDX, IDC_COMBO_OVERWRITE, m_overwritesel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConfigServer, CDialog)
	//{{AFX_MSG_MAP(CConfigServer)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_UNDO_ALL, OnUndoAll)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigServer message handlers


void CConfigServer::GetConfig()
{
	if (!m_pDev || m_bInited)
	{
		return;
	}

	DWORD retlen = 0;
	BOOL bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_DEVICECFG, 0,
								&m_sysCFG, sizeof(DHDEV_SYSTEM_ATTR_CFG), &retlen, CONFIG_WAITTIME);
	if (!bRet || retlen != sizeof(DHDEV_SYSTEM_ATTR_CFG))
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		m_bInited = TRUE;
	}
	//show config information
	CString strVer;
	strVer.Format("Software Version: %d.%d, Build:%d-%d-%d",
				m_sysCFG.stVersion.dwSoftwareVersion>>16, m_sysCFG.stVersion.dwSoftwareVersion & 0x0000FFFF,
				m_sysCFG.stVersion.dwSoftwareBuildDate>>16, (m_sysCFG.stVersion.dwSoftwareBuildDate>>8) & 0x000000FF,
				m_sysCFG.stVersion.dwSoftwareBuildDate & 0x000000FF
				);
	SetDlgItemText(IDC_VERSION, strVer);

	char *devNameList[25] = {"NONE","NoRealtimeMace","NoRealTime","MPEG1_1","MPEG1_2",
		"MPEG1_8","MPEG4_8","MPEG4_16","MPEG4_LB","MPEG4_GB",
		"MPEG4_HB","NPEG4_GBE","MPEG4_NVSII","STD_NEW","DDNS", "ATM",
		"NB", "LN","BAV","SDIP", "IPC", "NVS_B", "NET_C","NET_S", "NET_E"};

	SetDlgItemText(IDC_EDIT_DEVTYPE, devNameList[m_sysCFG.byDevType]);
	SetDlgItemText(IDC_EDIT_DEVTYPE2, (char *)m_sysCFG.szDevType);
	SetDlgItemText(IDC_EDIT_SERIALNO, (char*)m_sysCFG.szDevSerialNo);
	SetDlgItemInt(IDC_EDIT_VIDEONUM, m_sysCFG.byVideoCaptureNum);
	SetDlgItemInt(IDC_EDIT_AUDIONUM, m_sysCFG.byAudioCaptureNum);
	SetDlgItemInt(IDC_EDIT_ALARMIN, m_sysCFG.byAlarmInNum);
	SetDlgItemInt(IDC_EDIT_ALARMOUT, m_sysCFG.byAlarmOutNum);
	SetDlgItemInt(IDC_EDIT_NETPORT, m_sysCFG.byNetIONum);
	SetDlgItemInt(IDC_EDIT_USBNUM, m_sysCFG.byUsbIONum);
	SetDlgItemInt(IDC_EDIT_IDENUM, m_sysCFG.byIdeIONum);
	SetDlgItemInt(IDC_EDIT_LPORTNUM, m_sysCFG.byLPTIONum);
	SetDlgItemInt(IDC_EDIT_232NUM, m_sysCFG.byComIONum);
	SetDlgItemInt(IDC_EDIT_VGANUM, m_sysCFG.byVgaIONum);
	SetDlgItemInt(IDC_EDIT_TALKIN, m_sysCFG.byTalkInChanNum);
	SetDlgItemInt(IDC_EDIT_TALKOUT, m_sysCFG.byTalkOutChanNum);
	SetDlgItemInt(IDC_EDIT_DECCHAN, m_sysCFG.byDecodeChanNum);
	SetDlgItemInt(IDC_EDIT_IDECTRLNUM, m_sysCFG.byIdeControlNum);
	SetDlgItemInt(IDC_EDIT_IDECTRLTYPE, m_sysCFG.byIdeControlType);
	SetDlgItemInt(IDC_EDIT_DEVNO, m_sysCFG.wDevNo);
	m_overwritesel.SetCurSel(m_sysCFG.byOverWrite);
	SetDlgItemInt(IDC_EDIT_RECLEN, m_sysCFG.byRecordLen);
	m_videofmtsel.SetCurSel(m_sysCFG.byVideoStandard);
	m_datefmtsel.SetCurSel(m_sysCFG.byDateFormat);
	m_datesprsel.SetCurSel(m_sysCFG.byDateSprtr);
	m_timefmtsel.SetCurSel(m_sysCFG.byTimeFmt);
	SetDlgItemInt(IDC_EDIT_STARTCHAN, 0/*m_sysCFG.byStartChanNo*/);
}

void CConfigServer::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

/************************************************************************
 *	系统信息
************************************************************************/


BOOL CConfigServer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigServer::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow)
	{
		GetConfig();
	}
}

void CConfigServer::OnUndoAll() 
{
	m_bInited = FALSE;
	GetConfig();
}

void CConfigServer::OnApply() 
{
	if (!m_bInited)
	{
		return;
	}
	m_sysCFG.wDevNo = GetDlgItemInt(IDC_EDIT_DEVNO);
	m_sysCFG.byOverWrite = m_overwritesel.GetCurSel();
	m_sysCFG.byRecordLen = GetDlgItemInt(IDC_EDIT_RECLEN);
	m_sysCFG.byVideoStandard = m_videofmtsel.GetCurSel();
	m_sysCFG.byDateFormat = m_datefmtsel.GetCurSel();
	m_sysCFG.byDateSprtr = m_datesprsel.GetCurSel();
	m_sysCFG.byTimeFmt = m_timefmtsel.GetCurSel();

	BOOL bRet = CLIENT_SetDevConfig(m_pDev->LoginID, DH_DEV_DEVICECFG, 0,
								&m_sysCFG, sizeof(DHDEV_SYSTEM_ATTR_CFG), CONFIG_WAITTIME);
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
