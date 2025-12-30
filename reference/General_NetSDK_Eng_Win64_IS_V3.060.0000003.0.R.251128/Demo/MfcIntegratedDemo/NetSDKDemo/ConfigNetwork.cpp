// ConfigNetwork.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigNetwork.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigNetwork dialog


CConfigNetwork::CConfigNetwork(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigNetwork::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigNetwork)
	//}}AFX_DATA_INIT
	m_pDev = 0;
	memset(&m_netCFG, 0, sizeof(DHDEV_NET_CFG));
	memset(&m_sysCFG, 0, sizeof(DHDEV_SYSTEM_ATTR_CFG));
	//Begin: Add by li_deming(11517) 2008-1-30
	memset(&m_ddnsCFG,0,sizeof(DHDEV_MULTI_DDNS_CFG));
	//End:li_deming(11517)	
	m_bInited = FALSE;
	m_etherIdx = -1;
	m_hostIdx = -1;

	selIndexOld = -1;
}


void CConfigNetwork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigNetwork)
	DDX_Control(pDX, IDC_COMBO_DDNSID, m_ctlDDNSIDList);
	DDX_Control(pDX, IDC_CHECK_DDNSENABLE, m_ddnsEnable);
	DDX_Control(pDX, IDC_CHECK_REMOTEHOST, m_hostEnchk);
	DDX_Control(pDX, IDC_COMBO_NETINTERFACE, m_interfacesel);
	DDX_Control(pDX, IDC_COMBO_REMOTEHOST, m_remotehostsel);
	DDX_Control(pDX, IDC_COMBO_ETHERNET, m_ethernetsel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigNetwork, CDialog)
	//{{AFX_MSG_MAP(CConfigNetwork)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_UNDO_ALL, OnUndoAll)
	ON_CBN_SELCHANGE(IDC_COMBO_REMOTEHOST, OnSelchangeComboRemotehost)
	ON_CBN_SELCHANGE(IDC_COMBO_ETHERNET, OnSelchangeComboEthernet)
	ON_CBN_SELCHANGE(IDC_COMBO_DDNSID, OnSelchangeComboDdnsid)
	ON_CBN_SELENDOK(IDC_COMBO_DDNSID, OnSelendokComboDdnsid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigNetwork message handlers

BOOL CConfigNetwork::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);

	//ethernet type
	m_interfacesel.InsertString(0, ConvertString(NAME_NETCFG_NET1));
	m_interfacesel.InsertString(1, ConvertString(NAME_NETCFG_NET2));
	m_interfacesel.InsertString(2, ConvertString(NAME_NETCFG_NET3));
	m_interfacesel.InsertString(3, ConvertString(NAME_NETCFG_NET4));
	m_interfacesel.InsertString(4, ConvertString(NAME_NETCFG_NET5));

	//remote host list
	m_remotehostsel.InsertString(0, ConvertString(NAME_NETCFG_REMOTE_ALARM));
	m_remotehostsel.InsertString(1, ConvertString(NAME_NETCFG_REMOTE_LOG));
	m_remotehostsel.InsertString(2, ConvertString(NAME_NETCFG_REMOTE_SMTP));
	m_remotehostsel.InsertString(3, ConvertString(NAME_NETCFG_REMOTE_MULTICAST));
	m_remotehostsel.InsertString(4, ConvertString(NAME_NETCFG_REMOTE_NFS));
	m_remotehostsel.InsertString(5, ConvertString(NAME_NETCFG_REMOTE_PPPOE));
	m_remotehostsel.InsertString(6, ConvertString(NAME_NETCFG_REMOTE_DDNS));
	m_remotehostsel.InsertString(7, ConvertString(NAME_NETCFG_REMOTE_DNS));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigNetwork::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	GetConfig();
}

void CConfigNetwork::GetConfig()
{
	if (!m_pDev || m_bInited)
	{
		return;
	}
	
	while (m_ethernetsel.GetCount() > 0)
	{
		m_ethernetsel.DeleteString(0);
	}

	BOOL bRet = FALSE;
	DWORD retlen = 0;
	bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_DEVICECFG, 0,
								&m_sysCFG, sizeof(DHDEV_SYSTEM_ATTR_CFG), &retlen, CONFIG_WAITTIME*4);
	if (!bRet || retlen != sizeof(DHDEV_SYSTEM_ATTR_CFG)) 
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	CString strEth;
	for (int i = 0; i < m_sysCFG.byNetIONum; i++)
	{
		strEth.Format(ConvertString("Ethernet %d"), i+1);
		m_ethernetsel.InsertString(i, strEth);
	}

	bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_NETCFG, 0,
								&m_netCFG, sizeof(DHDEV_NET_CFG), &retlen, CONFIG_WAITTIME*4);
	if (!bRet || retlen != sizeof(DHDEV_NET_CFG))
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		m_bInited = TRUE;
	}

	//show config information
	SetDlgItemText(IDC_EDIT_DEVNAME, m_netCFG.sDevName);
	SetDlgItemInt(IDC_EDIT_MAXTCP, m_netCFG.wTcpMaxConnectNum);
	SetDlgItemInt(IDC_EDIT_TCPPORT, m_netCFG.wTcpPort);
	SetDlgItemInt(IDC_EDIT_UDPPORT, m_netCFG.wUdpPort);
	SetDlgItemInt(IDC_EDIT_HTTPPORT, m_netCFG.wHttpPort);
	SetDlgItemInt(IDC_EDIT_HTTPSPORT, m_netCFG.wHttpsPort);
	SetDlgItemInt(IDC_EDIT_SSLPORT, m_netCFG.wSslPort);
	SetDlgItemInt(IDC_EDIT_MAXTCP, m_netCFG.wTcpMaxConnectNum);
	SetDlgItemInt(IDC_EDIT_MAXTCP, m_netCFG.wTcpMaxConnectNum);
	
	//ethernet config
	if (m_ethernetsel.GetCount() > 0)
	{
		m_ethernetsel.SetCurSel(0);
		OnSelchangeComboEthernet();
	}

	//remote config
	if (m_remotehostsel.GetCount() > 0)
	{
		m_remotehostsel.SetCurSel(0);
		OnSelchangeComboRemotehost();
	}

	//mail config
	SetDlgItemText(IDC_EDIT_MAILUSERNAME, m_netCFG.struMail.sUserName);
	SetDlgItemText(IDC_EDIT_MAILUSERPSW, m_netCFG.struMail.sUserPsw);
	SetDlgItemText(IDC_EDIT_MAILIP, m_netCFG.struMail.sMailIPAddr);
	SetDlgItemInt(IDC_EDIT_MAILPORT, m_netCFG.struMail.wMailPort);
	SetDlgItemText(IDC_EDIT_MAILDESADDR, m_netCFG.struMail.sDestAddr);
	SetDlgItemText(IDC_EDIT_MAILCCADDR, m_netCFG.struMail.sCcAddr);
	SetDlgItemText(IDC_EDIT_MAILBCCADDR, m_netCFG.struMail.sBccAddr);
	SetDlgItemText(IDC_EDIT_MAILSUBJECT, m_netCFG.struMail.sSubject);
	
	//Begin: Add by li_deming(11517) 2008-1-30
	//DDNS config
	bRet = FALSE;
	bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_MULTI_DDNS, 0,
								&m_ddnsCFG, sizeof(DHDEV_MULTI_DDNS_CFG), &retlen, CONFIG_WAITTIME*4);
	selIndexOld=-1;	
	if(bRet == TRUE)
	{
		CString strItem="";
		m_ctlDDNSIDList.Clear();
		for(int i=0; i<m_ddnsCFG.dwDdnsServerNum;i++)
		{
			strItem="";
			strItem.Format(ConvertString("DDNS %d"),i);
			m_ctlDDNSIDList.InsertString( i,  strItem);				
		}
		UpdateData(FALSE);
		m_ctlDDNSIDList.SetCurSel(0);
		OnSelchangeComboDdnsid();
	}
	else
	{
		AfxMessageBox(ConvertString("Get Dev Multi DDNS Info Fail!"));
	}
	
	//End:li_deming(11517)
}

void CConfigNetwork::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

DH_REMOTE_HOST *CConfigNetwork::GetRemoteHostType(int hostIdx, BOOL bChg)
{
	DH_REMOTE_HOST *ret = 0;
	switch(hostIdx)
	{
	case 0:	/* 报警服务器 */
		ret = &m_netCFG.struAlarmHost;
		break;
	case 1:/* 日志服务器 */
		ret = &m_netCFG.struLogHost;
		break;
	case 2:/* SMTP服务器 */
		ret = &m_netCFG.struSmtpHost;
		break;
	case 3:/* 多播组 */
		ret = &m_netCFG.struMultiCast;
		break;
	case 4:/* NFS服务器 */
		ret = &m_netCFG.struNfs;
		break;
	case 5:/* PPPoE服务器 */
		ret = &m_netCFG.struPppoe;
		SetDlgItemText(IDC_HOSTRESERVED, ConvertString(NAME_NETCFG_PPPOE_IP));
		SetDlgItemText(IDC_EDIT_HOSTRESERVED, m_netCFG.sPppoeIP);
		break;
	case 6:/* DDNS服务器 */
		ret = &m_netCFG.struDdns;		
		if (bChg)
		{
			SetDlgItemText(IDC_HOSTRESERVED, ConvertString(NAME_NETCFG_DDNS_HOSTNAME));
			SetDlgItemText(IDC_EDIT_HOSTRESERVED, m_netCFG.sDdnsHostName);
		}
		break;
	case 7:/* DNS服务器*/
		ret = &m_netCFG.struDns;
		break;
	default:
		break;
	}
	
	if (bChg && ret)
	{
		m_hostEnchk.EnableWindow(0 == hostIdx || 5 == hostIdx || 6 == hostIdx);
		GetDlgItem(IDC_EDIT_HOSTIP)->EnableWindow(5 != hostIdx);
		GetDlgItem(IDC_EDIT_HOSTPORT)->EnableWindow(5 != hostIdx && 7 != hostIdx);
		GetDlgItem(IDC_EDIT_HOSTUSERNAME)->EnableWindow(5 == hostIdx);
		GetDlgItem(IDC_EDIT_HOSTUSERPSW)->EnableWindow(5 == hostIdx);
		GetDlgItem(IDC_HOSTRESERVED)->ShowWindow((5 == hostIdx || 6 == hostIdx)?SW_SHOW:SW_HIDE);
		GetDlgItem(IDC_EDIT_HOSTRESERVED)->ShowWindow((5 == hostIdx || 6 == hostIdx)?SW_SHOW:SW_HIDE);
	}
	return ret;
}

void CConfigNetwork::OnApply() 
{
	if (!m_bInited)
	{
		return;
	}
	
	if (m_etherIdx < 0)
	{
		return;
	}
	
	if (m_hostIdx < 0)
	{
		return;
	}

	//get config information
	GetDlgItemText(IDC_EDIT_DEVNAME, m_netCFG.sDevName, DH_MAX_NAME_LEN);
	m_netCFG.wTcpMaxConnectNum = GetDlgItemInt(IDC_EDIT_MAXTCP);
	m_netCFG.wTcpPort = GetDlgItemInt(IDC_EDIT_TCPPORT);
	m_netCFG.wUdpPort = GetDlgItemInt(IDC_EDIT_UDPPORT);
	m_netCFG.wHttpPort = GetDlgItemInt(IDC_EDIT_HTTPPORT);
	m_netCFG.wHttpsPort = GetDlgItemInt(IDC_EDIT_HTTPSPORT);
	m_netCFG.wSslPort = GetDlgItemInt(IDC_EDIT_SSLPORT);
	m_netCFG.wTcpMaxConnectNum = GetDlgItemInt(IDC_EDIT_MAXTCP);
	m_netCFG.wTcpMaxConnectNum = GetDlgItemInt(IDC_EDIT_MAXTCP);
	
	//ethernet config
	GetDlgItemText(IDC_EDIT_NETIP, m_netCFG.stEtherNet[m_etherIdx].sDevIPAddr, DH_MAX_IPADDR_LEN);
	GetDlgItemText(IDC_EDIT_NETIPMASK, m_netCFG.stEtherNet[m_etherIdx].sDevIPMask, DH_MAX_IPADDR_LEN);
	GetDlgItemText(IDC_EDIT_NETGATEWAY, m_netCFG.stEtherNet[m_etherIdx].sGatewayIP, DH_MAX_IPADDR_LEN);
	m_netCFG.stEtherNet[m_etherIdx].dwNetInterface = m_interfacesel.GetCurSel()+1;
	GetDlgItemText(IDC_EDIT_NETMAC, m_netCFG.stEtherNet[m_etherIdx].byMACAddr, DH_MACADDR_LEN);
	
	//remote config
	DH_REMOTE_HOST *thishost = 0;
	thishost = GetRemoteHostType(m_hostIdx);

	if (!thishost)
	{
		return;
	}
	
	thishost->byEnable = m_hostEnchk.GetCheck();
	GetDlgItemText(IDC_EDIT_HOSTIP, thishost->sHostIPAddr, DH_MAX_IPADDR_LEN);
	thishost->wHostPort = GetDlgItemInt(IDC_EDIT_HOSTPORT);
	GetDlgItemText(IDC_EDIT_HOSTUSERNAME, thishost->sHostUser, DH_MAX_HOST_NAMELEN);
	GetDlgItemText(IDC_EDIT_HOSTUSERPSW, thishost->sHostPassword, DH_MAX_HOST_PSWLEN);
	if (7 == m_hostIdx)
	{
		GetDlgItemText(IDC_EDIT_HOSTRESERVED, m_netCFG.sDdnsHostName, DH_MAX_HOST_NAMELEN);
	}
	
	//mail config
	GetDlgItemText(IDC_EDIT_MAILUSERNAME, m_netCFG.struMail.sUserName, DH_MAX_NAME_LEN);
	GetDlgItemText(IDC_EDIT_MAILUSERPSW, m_netCFG.struMail.sUserPsw, DH_MAX_NAME_LEN);
	GetDlgItemText(IDC_EDIT_MAILIP, m_netCFG.struMail.sMailIPAddr, DH_MAX_IPADDR_LEN);
	m_netCFG.struMail.wMailPort = GetDlgItemInt(IDC_EDIT_MAILPORT);
	GetDlgItemText(IDC_EDIT_MAILDESADDR, m_netCFG.struMail.sDestAddr, DH_MAX_MAIL_ADDR_LEN);
	GetDlgItemText(IDC_EDIT_MAILCCADDR, m_netCFG.struMail.sCcAddr, DH_MAX_MAIL_ADDR_LEN);
	GetDlgItemText(IDC_EDIT_MAILBCCADDR, m_netCFG.struMail.sBccAddr, DH_MAX_MAIL_ADDR_LEN);
	GetDlgItemText(IDC_EDIT_MAILSUBJECT, m_netCFG.struMail.sSubject, DH_MAX_MAIL_ADDR_LEN);
	
//	strcpy(m_netCFG.struMail.sSenderAddr, "111111111111");
//
	//save
	BOOL bRet = CLIENT_SetDevConfig(m_pDev->LoginID, DH_DEV_NETCFG, 0,
								&m_netCFG, sizeof(DHDEV_NET_CFG), CONFIG_WAITTIME);
	if (!bRet)
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();				
		return;
	}
	else
	{
		//Begin: Add by li_deming(11517) 2008-1-30
		//DDNS配置保存
		m_ctlDDNSIDList.SetCurSel(-1);
		OnSelchangeComboDdnsid();
		m_ctlDDNSIDList.SetCurSel(selIndexOld);
		OnSelchangeComboDdnsid();
		bRet=CLIENT_SetDevConfig(m_pDev->LoginID,DH_DEV_MULTI_DDNS,0,&m_ddnsCFG,sizeof(DHDEV_MULTI_DDNS_CFG),CONFIG_WAITTIME);
		if(bRet==TRUE)
		{
			MessageBox(ConvertString(MSG_CONFIG_SUCCESS), "OK");
		}
		else
		{
			((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();				
			return;
		}
		//End:li_deming(11517)		
		//essageBox(MSG_CONFIG_SUCCESS, "OK");//// Delete by li_deming(11517) 2008-1-30
	}

}

void CConfigNetwork::OnUndoAll() 
{
	m_bInited = FALSE;
	m_etherIdx = -1;
	m_hostIdx = -1;

	GetConfig();
}

void CConfigNetwork::OnSelchangeComboRemotehost() 
{
	if (!m_bInited)
	{
		return;
	}
	//store current config
	if (m_hostIdx >= 0)
	{
		DH_REMOTE_HOST *prehost = 0;
		prehost = GetRemoteHostType(m_hostIdx);
		if (!prehost)
		{
			return;
		}
		
		prehost->byEnable = m_hostEnchk.GetCheck();
		GetDlgItemText(IDC_EDIT_HOSTIP, prehost->sHostIPAddr, DH_MAX_IPADDR_LEN);
		prehost->wHostPort = GetDlgItemInt(IDC_EDIT_HOSTPORT);
		GetDlgItemText(IDC_EDIT_HOSTUSERNAME, prehost->sHostUser, DH_MAX_HOST_NAMELEN);
		GetDlgItemText(IDC_EDIT_HOSTUSERPSW, prehost->sHostPassword, DH_MAX_HOST_PSWLEN);
		if (7 == m_hostIdx)//DDNS配置
		{
			GetDlgItemText(IDC_EDIT_HOSTRESERVED, m_netCFG.sDdnsHostName, DH_MAX_HOST_NAMELEN);
		}
	}
	
	//show new config
	int hostIdx = m_remotehostsel.GetCurSel();
	if (hostIdx < 0 || hostIdx >= 9) 
	{
		return;
	}
	m_hostIdx = hostIdx;
	
	DH_REMOTE_HOST *thishost = 0;
	thishost = GetRemoteHostType(hostIdx, TRUE);
	if (!thishost)
	{
		return;
	}
	
	m_hostEnchk.SetCheck(thishost->byEnable?1:0);
	SetDlgItemText(IDC_EDIT_HOSTIP, thishost->sHostIPAddr);
	SetDlgItemInt(IDC_EDIT_HOSTPORT, thishost->wHostPort);
	SetDlgItemText(IDC_EDIT_HOSTUSERNAME, thishost->sHostUser);
	SetDlgItemText(IDC_EDIT_HOSTUSERPSW, thishost->sHostPassword);
}

void CConfigNetwork::OnSelchangeComboEthernet() 
{
	if (!m_bInited)
	{
		return;
	}
	//store current config
	if (m_etherIdx >= 0)
	{
		GetDlgItemText(IDC_EDIT_NETIP, m_netCFG.stEtherNet[m_etherIdx].sDevIPAddr, DH_MAX_IPADDR_LEN);
		GetDlgItemText(IDC_EDIT_NETIPMASK, m_netCFG.stEtherNet[m_etherIdx].sDevIPMask, DH_MAX_IPADDR_LEN);
		GetDlgItemText(IDC_EDIT_NETGATEWAY, m_netCFG.stEtherNet[m_etherIdx].sGatewayIP, DH_MAX_IPADDR_LEN);
		m_netCFG.stEtherNet[m_etherIdx].dwNetInterface = m_interfacesel.GetCurSel()+1;
		GetDlgItemText(IDC_EDIT_NETMAC, m_netCFG.stEtherNet[m_etherIdx].byMACAddr, DH_MACADDR_LEN);
	}
	//show new config
	int etherIdx = m_ethernetsel.GetCurSel();
	if (etherIdx < 0 || etherIdx >= DH_MAX_ETHERNET_NUM) 
	{
		return;
	}
	m_etherIdx = etherIdx;
	
	SetDlgItemText(IDC_EDIT_NETIP, m_netCFG.stEtherNet[etherIdx].sDevIPAddr);
	SetDlgItemText(IDC_EDIT_NETIPMASK, m_netCFG.stEtherNet[etherIdx].sDevIPMask);
	SetDlgItemText(IDC_EDIT_NETGATEWAY, m_netCFG.stEtherNet[etherIdx].sGatewayIP);
	m_interfacesel.SetCurSel(m_netCFG.stEtherNet[etherIdx].dwNetInterface-1);
	SetDlgItemText(IDC_EDIT_NETMAC, m_netCFG.stEtherNet[etherIdx].byMACAddr);
}

void CConfigNetwork::OnSelchangeComboDdnsid() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString strTemp;
	DH_DDNS_SERVER_CFG * dns;
	if(selIndexOld>=0)
	{	
		dns=&m_ddnsCFG.struDdnsServer[selIndexOld];
		dns->bEnable=m_ddnsEnable.GetCheck();
		GetDlgItemText(IDC_EDIT_ALIVEPERIOD,strTemp);
		dns->dwAlivePeriod=(DWORD)atoi(strTemp);
		GetDlgItemText(IDC_EDIT_SERVERPORT,strTemp);
		dns->dwServerPort=(DWORD)atoi(strTemp);
		GetDlgItemText(IDC_EDIT_ALIAS,dns->szAlias,DH_MAX_DDNS_ALIAS_LEN);
		GetDlgItemText(IDC_EDIT_DOMAINNAME,dns->szDomainName,DH_MAX_DOMAIN_NAME_LEN);
		GetDlgItemText(IDC_EDIT_SERVERIP,dns->szServerIp,DH_MAX_IPADDR_LEN);
		GetDlgItemText(IDC_EDIT_SERVRRTYPE,dns->szServerType,DH_MAX_SERVER_TYPE_LEN);
		//dns->szUserName=strTemp;
		//dns->szUserPsw=strTemp;
	}
	int selIndex=0;
	selIndex=m_ctlDDNSIDList.GetCurSel();
	if(selIndex<0)
	{
		return;
	}
	
	dns=&m_ddnsCFG.struDdnsServer[selIndex];
	SetDlgItemText(IDC_EDIT_SERVRRTYPE,dns->szServerType);
	SetDlgItemText(IDC_EDIT_SERVERIP,dns->szServerIp);
	strTemp.Format("%d",dns->dwServerPort);
	SetDlgItemText(IDC_EDIT_SERVERPORT,strTemp);
	SetDlgItemText(IDC_EDIT_DOMAINNAME,dns->szDomainName);
	SetDlgItemText(IDC_EDIT_ALIAS,dns->szAlias);
	strTemp.Format("%d",dns->dwAlivePeriod);
	SetDlgItemText(IDC_EDIT_ALIVEPERIOD,strTemp);
	m_ddnsEnable.SetCheck(dns->bEnable);
	UpdateData(FALSE);
	selIndexOld=selIndex;
}

void CConfigNetwork::OnSelendokComboDdnsid() 
{
	// TODO: Add your control notification handler code here
	//selIndexOld=m_ctlDDNSIDList.GetCurSel();
}
