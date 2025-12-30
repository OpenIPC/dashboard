// ServerConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ServerConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerConfig dialog


CServerConfig::CServerConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CServerConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerConfig)
	m_dvrName = _T("");
	m_alarmInNum = 0;
	m_alarmOutNum = 0;
	m_DNSIP = _T("");
	m_DSPVersion = _T("");
	m_dvrIP = _T("");
	m_dvrGatewayIP = _T("");
	m_hardwareVersion = _T("");
	m_MACAddr = _T("");
	m_manageHostIP = _T("");
	m_multicastIP = _T("");
	m_NASDir = _T("");
	m_NASIP = _T("");
	m_PPPOEIP = _T("");
	m_PPPOEUserName = _T("");
	m_serialNo = _T("");
	m_softwareVersion = _T("");
	m_manageHostPort = 0;
	m_httpPort = 0;
	m_dvrPort = 0;
	m_dvrIPMask = _T("");
	m_YKQID = 0;
	m_PPPOEPsw = _T("");
	m_panelVersion = 0;
	m_channelNum = 0;
	m_harddiskNum = 0;
	//}}AFX_DATA_INIT
}


void CServerConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerConfig)
	DDX_Control(pDX, IDC_SERVERTYPE, m_serverTypesel);
	DDX_Control(pDX, IDC_PPPOE, m_PPPOEsel);
	DDX_Control(pDX, IDC_CYCLERECORD, m_cycleRecordsel);
	DDX_Control(pDX, IDC_COMBO_INTERFACE, m_interfacesel);
	DDX_Text(pDX, IDC_DVR_NAME, m_dvrName);
	DDX_Text(pDX, IDC_ALARM_INNUM, m_alarmInNum);
	DDX_Text(pDX, IDC_ALARM_OUTNUM, m_alarmOutNum);
	DDX_Text(pDX, IDC_DNSIP, m_DNSIP);
	DDX_Text(pDX, IDC_DSPVERSION, m_DSPVersion);
	DDX_Text(pDX, IDC_DVR_IP, m_dvrIP);
	DDX_Text(pDX, IDC_GATEWAYIP, m_dvrGatewayIP);
	DDX_Text(pDX, IDC_HARDVERSION, m_hardwareVersion);
	DDX_Text(pDX, IDC_MACADDR, m_MACAddr);
	DDX_Text(pDX, IDC_MANAGEHOSTIP, m_manageHostIP);
	DDX_Text(pDX, IDC_MULTICASTIP, m_multicastIP);
	DDX_Text(pDX, IDC_NASDIR, m_NASDir);
	DDX_Text(pDX, IDC_NASIP, m_NASIP);
	DDX_Text(pDX, IDC_PPPOE_IP, m_PPPOEIP);
	DDX_Text(pDX, IDC_PPPOE_USERNAME, m_PPPOEUserName);
	DDX_Text(pDX, IDC_SERIALNUM, m_serialNo);
	DDX_Text(pDX, IDC_SOFTVERSION, m_softwareVersion);
	DDX_Text(pDX, IDC_MANAGEHOSTPORT, m_manageHostPort);
	DDX_Text(pDX, IDC_HTTPPORT, m_httpPort);
	DDX_Text(pDX, IDC_DVR_PORT, m_dvrPort);
	DDX_Text(pDX, IDC_DVR_IPMASK, m_dvrIPMask);
	DDX_Text(pDX, IDC_DVR_REMOTEID, m_YKQID);
	DDX_Text(pDX, IDC_PPPOE_PSW, m_PPPOEPsw);
	DDX_Text(pDX, IDC_PANELVERSION, m_panelVersion);
	DDX_Text(pDX, IDC_CHANNELNUM, m_channelNum);
	DDX_Text(pDX, IDC_HARDDISKNUM, m_harddiskNum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServerConfig, CDialog)
	//{{AFX_MSG_MAP(CServerConfig)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_RESTORE, OnRestore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerConfig message handlers
BOOL checkIPStr(CString ip)
{	//检查IP地址输入的正确性
	char str[20];
	strcpy(str,ip);
    char a;
    int dot=0;
    int a3,a2,a1,a0,i = 0;
	a3 = a2 = a1 = a0 = -1;
    if(strlen(str)==0)
		return TRUE;
    while(a=str[i++])
	{
        if((a==' ')||(a=='.')||((a>='0')&&(a<='9')))
		{
            if(a=='.')
                dot++;
        }
        else
			return FALSE;
    }
    if(dot!=3)
		return FALSE;
    else
	{
        sscanf(str,"%d.%d.%d.%d",&a3,&a2,&a1,&a0);
        if( (a0>255)||(a1>255)||(a2>255)||(a3>255))
			return FALSE;
		if( (a0 < 0) || (a1 < 0) || (a2 < 0) || (a3 < 0) )
			return FALSE;
    }
	return TRUE;
}

void CServerConfig::OnApply() 
{

	if (!UpdateData(TRUE))
	{
		return;
	}
	
		if(m_dvrPort <= 0)
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALPORT);
		return;
	}
	
	if( (m_dvrIP == "0.0.0.0") || (m_dvrIP == "") || !checkIPStr(m_dvrIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALIP);
		return;
	}
	
	if(!checkIPStr(m_dvrIPMask))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALMASK);
		return;
	}
	
	if(!checkIPStr(m_DNSIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALDNSIP);
		return;
	}
	if(!checkIPStr(m_multicastIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALMULTICASTIP);
		return;
	}
	if(!checkIPStr(m_dvrGatewayIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALGATEWAY);
		return;
	}
	if(!checkIPStr(m_NASIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALNASIP);
		return;
	}
	if(!checkIPStr(m_manageHostIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALMANAGEIP);
		return;
	}
	if(!checkIPStr(m_PPPOEIP))
	{
		AfxMessageBox(MSG_SERIALCFG_ILLEGALPPPOEIP);
		return;
	}
	
	m_deviceCFG.dwSize = sizeof(NET_DEV_DEVICECFG);
	m_deviceCFG.dwDVRID = m_YKQID;
	memcpy(m_deviceCFG.sDVRName, m_dvrName, NAME_LEN);	
	memcpy(m_netCFG.struEtherNet[0].sDVRIP, m_dvrIP, 16);
	memcpy(m_netCFG.struEtherNet[0].sDVRIPMask, m_dvrIPMask, 16);
	memcpy(m_netCFG.sDNSIP, m_DNSIP, 16);	
	memcpy(m_netCFG.sMultiCastIP, m_multicastIP,16);
	memcpy(m_netCFG.sGatewayIP, m_dvrGatewayIP,16);
	m_netCFG.struEtherNet[0].wDVRPort = m_dvrPort;
	memcpy(m_netCFG.sNFSDirectory, m_NASDir,PATHNAME_LEN);
	memcpy(m_netCFG.sNFSIP, m_NASIP,16);
	memcpy(m_netCFG.sManageHostIP, m_manageHostIP,16);
	m_netCFG.wManageHostPort = m_manageHostPort;
	memcpy(m_netCFG.sPPPoEUser, m_PPPOEUserName, NAME_LEN);
	memcpy(m_netCFG.sPPPoEPassword, m_PPPOEPsw, PASSWD_LEN);
	memcpy(m_netCFG.sPPPoEIP, m_PPPOEIP, 16);
	m_netCFG.struEtherNet[0].dwNetInterface = m_interfacesel.GetCurSel()+1;
	m_deviceCFG.dwRecycleRecord = m_cycleRecordsel.GetCurSel();
	
	m_netCFG.dwSize = sizeof(NET_DEV_NETCFG);
	m_netCFG.dwPPPOE = m_PPPOEsel.GetCurSel();
	m_netCFG.wHttpPort = m_httpPort;

	if(!CLIENT_SetDevConfig(m_lLoginID, DEV_SET_DEVICECFG, 0,&m_deviceCFG, sizeof(NET_DEV_DEVICECFG), CONFIG_WAITTIME))
	{
		AfxMessageBox(MSG_SERIALCFG_SAVECFGFAILED);
		return ;
	}
	if(!CLIENT_SetDevConfig(m_lLoginID, DEV_SET_NETCFG, 0,&m_netCFG, sizeof(NET_DEV_NETCFG), CONFIG_WAITTIME))
	{
		AfxMessageBox(MSG_SERIALCFG_SAVECFGFAILED);	
		return ;
	}
	//bSetServerParams = TRUE;

}

void CServerConfig::OnRestore() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CServerConfig::OnInitDialog() 
{
	
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);
	
	//界面初始化
	m_interfacesel.InsertString(0, MSG_SERVERCFG_10M_T);
	m_interfacesel.InsertString(1, MSG_SERVERCFG_10M_TFULL);
	m_interfacesel.InsertString(2, MSG_SERVERCFG_100M_TX);
	m_interfacesel.InsertString(3, MSG_SERVERCFG_100M_FULL);
	m_interfacesel.InsertString(4, MSG_SERVERCFG_10_100M);

	m_cycleRecordsel.InsertString(0, MSG_SERVERCFG_YES);	
	m_cycleRecordsel.InsertString(1, MSG_SERVERCFG_NO);

	m_PPPOEsel.InsertString(0, MSG_SERVERCFG_DEACTIVE);
	m_PPPOEsel.InsertString(1, MSG_SERVERCFG_ACTIVE);
	
	
	
	


	char cTemp[200];

//	bSetServerParams = FALSE;
//	m_bConnectServer = TRUE;
	
	DWORD dwReturned;

	memset(&m_deviceCFG, 0, sizeof(NET_DEV_DEVICECFG));
	if(!CLIENT_GetDevConfig(m_lLoginID, DEV_GET_DEVICECFG, 0,&m_deviceCFG, sizeof(NET_DEV_DEVICECFG), &dwReturned, CONFIG_WAITTIME))
	{
		MessageBox("AAAA");
	} 
	
	if(!CLIENT_GetDevConfig(m_lLoginID, DEV_GET_NETCFG, 0,&m_netCFG, sizeof(NET_DEV_NETCFG), &dwReturned, CONFIG_WAITTIME))
	{
		MessageBox("BBBB");
	}
	ZeroMemory(cTemp, 200);
	memcpy(cTemp, m_deviceCFG.sDVRName, NAME_LEN);
	m_dvrName.Format("%s", cTemp);
	m_YKQID = m_deviceCFG.dwDVRID;
	ZeroMemory(cTemp, 200);
	memcpy(cTemp, m_deviceCFG.sSerialNumber, SERIALNO_LEN);
	m_serialNo.Format("%s", cTemp);
	m_softwareVersion.Format("V%d.%d build %02d%02d%02d", (m_deviceCFG.dwSoftwareVersion>>16)&0xFFFF, m_deviceCFG.dwSoftwareVersion&0xFFFF, (m_deviceCFG.dwSoftwareBuildDate>>16)&0xFFFF, (m_deviceCFG.dwSoftwareBuildDate>>8)&0xFF,m_deviceCFG.dwSoftwareBuildDate&0xFF);
	m_hardwareVersion.Format("0x%x", m_deviceCFG.dwHardwareVersion);
	m_panelVersion = m_deviceCFG.dwPanelVersion;
	m_DSPVersion.Format("V%d.%d build %02d%02d%02d", (m_deviceCFG.dwDSPSoftwareVersion>>16)&0xFFFF, m_deviceCFG.dwDSPSoftwareVersion&0xFFFF, (m_deviceCFG.dwDSPSoftwareBuildDate>>16)&0xFFFF-2000, (m_deviceCFG.dwDSPSoftwareBuildDate>>8)&0xFF,m_deviceCFG.dwDSPSoftwareBuildDate&0xFF);
	m_alarmInNum = m_deviceCFG.byAlarmInPortNum;
	m_alarmOutNum = m_deviceCFG.byAlarmOutPortNum;
	m_channelNum = m_deviceCFG.byChanNum;	
	m_serverTypesel.SetCurSel(m_deviceCFG.dwDVRType - 1);
	m_DNSIP = m_netCFG.sDNSIP;
	m_dvrGatewayIP = m_netCFG.sGatewayIP;
	m_dvrIP = m_netCFG.struEtherNet[0].sDVRIP;
	m_dvrIPMask = m_netCFG.struEtherNet[0].sDVRIPMask;
	m_MACAddr.Format("%02x:%02x:%02x:%02x:%02x:%02x", m_netCFG.struEtherNet[0].byMACAddr[0],m_netCFG.struEtherNet[0].byMACAddr[1],m_netCFG.struEtherNet[0].byMACAddr[2],m_netCFG.struEtherNet[0].byMACAddr[3],m_netCFG.struEtherNet[0].byMACAddr[4],m_netCFG.struEtherNet[0].byMACAddr[5]);
	m_multicastIP = m_netCFG.sMultiCastIP;
	ZeroMemory(cTemp, 200);
	memcpy(cTemp, m_netCFG.sNFSDirectory, PATHNAME_LEN);
	m_NASDir.Format("%s", cTemp);
	m_NASIP = m_netCFG.sNFSIP;
	m_dvrPort = m_netCFG.struEtherNet[0].wDVRPort;
	m_harddiskNum = m_deviceCFG.byDiskNum;
	m_manageHostIP = m_netCFG.sManageHostIP;
	m_manageHostPort = m_netCFG.wManageHostPort;
	ZeroMemory(cTemp, 200);
	memcpy(cTemp, m_netCFG.sPPPoEUser, NAME_LEN);
	m_PPPOEUserName.Format("%s", cTemp);
	ZeroMemory(cTemp, 200);
	memcpy(cTemp, m_netCFG.sPPPoEPassword, PASSWD_LEN);
	m_PPPOEPsw.Format("%s", cTemp);
	
	m_cycleRecordsel.SetCurSel(m_deviceCFG.dwRecycleRecord);
	m_interfacesel.SetCurSel(m_netCFG.struEtherNet[0].dwNetInterface-1);
	m_PPPOEsel.SetCurSel(m_netCFG.dwPPPOE);
	m_PPPOEIP = m_netCFG.sPPPoEIP;
	m_httpPort = m_netCFG.wHttpPort;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServerConfig::SetLoginID(LONG lLoginID)
{
	m_lLoginID = lLoginID;
}
