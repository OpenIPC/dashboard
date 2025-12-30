// DlgCfgNetwork.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgNetwork.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetwork dialog


CDlgCfgNetwork::CDlgCfgNetwork(CWnd* pParent /*=NULL*/, LLONG lLoginID)
	: CDialog(CDlgCfgNetwork::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgNetwork)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginID = lLoginID;
	//}}AFX_DATA_INIT
	memset(&m_stuNetwork, 0, sizeof(m_stuNetwork));
}


void CDlgCfgNetwork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgNetwork)
	DDX_Control(pDX, IDC_NETWORK_MASK, m_ctlMask);
	DDX_Control(pDX, IDC_NETWORK_IP, m_ctlIp);
	DDX_Control(pDX, IDC_NETWORK_GATEWAY, m_ctlGateway);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgNetwork, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgNetwork)
	ON_BN_CLICKED(IDC_NETWORK_BTN_CANCEL, OnBtnCancel)
	ON_BN_CLICKED(IDC_NETWORK_BTN_SET, OnBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetwork message handlers

BOOL CDlgCfgNetwork::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_NETWORK);
	
	// TODO: Add extra initialization here
	m_ctlIp.SetAddress(0, 0, 0, 0);
	m_ctlMask.SetAddress(0, 0, 0, 0);
	m_ctlGateway.SetAddress(0, 0, 0, 0);

	GetNetworkPara();
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgNetwork::OnBtnCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CDlgCfgNetwork::OnBtnSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if (0 == m_lLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_NETWORK), ConvertString("Prompt"));
		return;
	}

	CString csIp, csMask, csGateway;
	m_ctlIp.GetWindowText(csIp);
	m_ctlMask.GetWindowText(csMask);
	m_ctlGateway.GetWindowText(csGateway);

//	CFG_NETWORK_INFO stuNetworkPara = {0};
/*	CFG_NETWORK_INFO& stuNetworkPara = m_stuNetwork;
	stuNetworkPara.nInterfaceNum = 1;
	strncpy(stuNetworkPara.stuInterfaces[0].szIP, csIp, MAX_ADDRESS_LEN-1);
	strncpy(stuNetworkPara.stuInterfaces[0].szSubnetMask, csMask, MAX_ADDRESS_LEN-1);
	strncpy(stuNetworkPara.stuInterfaces[0].szDefGateway, csGateway, MAX_ADDRESS_LEN-1);*/

	CFG_NETWORK_INFO& stuNetworkPara = m_stuNetwork;
	for (int i=0;i<stuNetworkPara.nInterfaceNum;i++)
	{
		if (strcmp("eth0", stuNetworkPara.stuInterfaces[i].szName) == 0)
		{
			strncpy(stuNetworkPara.stuInterfaces[i].szIP, csIp, MAX_ADDRESS_LEN-1);
			strncpy(stuNetworkPara.stuInterfaces[i].szSubnetMask, csMask, MAX_ADDRESS_LEN-1);
			strncpy(stuNetworkPara.stuInterfaces[i].szDefGateway, csGateway, MAX_ADDRESS_LEN-1);
		}
	}

	char szJsonBuf[1024*4] = {0};
	BOOL nRet = CLIENT_PacketData(CFG_CMD_NETWORK, &stuNetworkPara, sizeof(CFG_NETWORK_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!nRet)
	{
		MessageBox(ConvertString("Packet network json buffer failed.", DLG_CFG_NETWORK), ConvertString("Prompt"));
		return;
	}

	int nErr = 0, nRestart = 0;
	nRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_NETWORK,
		-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!nRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("SetupConfig network failed:", DLG_CFG_NETWORK), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return ;
	}

 	MessageBox(ConvertString("send network parameters successfully.", DLG_CFG_NETWORK), ConvertString("Prompt"));

	return;
}

void CDlgCfgNetwork::GetNetworkPara()
{
	if (0 == m_lLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_NETWORK), ConvertString("Prompt"));
		return;
	}
	char szJsonBuf[1024*4] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_NETWORK,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("GetConfig network failed:", DLG_CFG_NETWORK), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	}

	CFG_NETWORK_INFO stNetworkInfo = {0};
	DWORD dwRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_NETWORK, szJsonBuf, &stNetworkInfo, sizeof(stNetworkInfo), &dwRetLen);
	if (!bRet)
	{
		MessageBox(ConvertString("Parse network jason failed.", DLG_CFG_NETWORK), ConvertString("Prompt"));
		return;
	}
/*
	if (0 == stNetworkInfo.stuInterfaces[0].szIP[0] 
		|| 0 == stNetworkInfo.stuInterfaces[0].szSubnetMask[0]
		|| 0 == stNetworkInfo.stuInterfaces[0].szDefGateway[0])
	{
		MessageBox(ConvertString("Invalidate ip parameters.", DLG_CFG_NETWORK), ConvertString("Prompt"));
		return;
	}

	memcpy(&m_stuNetwork, &stNetworkInfo, sizeof(CFG_NETWORK_INFO));

	m_ctlIp.SetWindowText(stNetworkInfo.stuInterfaces[0].szIP);
	m_ctlMask.SetWindowText(stNetworkInfo.stuInterfaces[0].szSubnetMask);
	m_ctlGateway.SetWindowText(stNetworkInfo.stuInterfaces[0].szDefGateway);*/


	for (int i=0;i<stNetworkInfo.nInterfaceNum;i++)
	{
		if (strcmp(stNetworkInfo.szDefInterface, stNetworkInfo.stuInterfaces[i].szName) == 0)
		{
			if (0 == stNetworkInfo.stuInterfaces[i].szIP[0] 
			|| 0 == stNetworkInfo.stuInterfaces[i].szSubnetMask[0]
			|| 0 == stNetworkInfo.stuInterfaces[i].szDefGateway[0])
			{
				MessageBox(ConvertString("Invalidate ip parameters.", DLG_CFG_NETWORK), ConvertString("Prompt"));
				return;
			}
			else
			{
				memcpy(&m_stuNetwork, &stNetworkInfo, sizeof(CFG_NETWORK_INFO));
				m_ctlIp.SetWindowText(stNetworkInfo.stuInterfaces[i].szIP);
				m_ctlMask.SetWindowText(stNetworkInfo.stuInterfaces[i].szSubnetMask);
				m_ctlGateway.SetWindowText(stNetworkInfo.stuInterfaces[i].szDefGateway);
				return;
			}
		}
	}

#if 0
	DWORD dwIp = ntohl(inet_addr(stNetworkInfo.stuInterfaces[0].szIP));
	m_ctlIp.SetAddress(dwIp);

	DWORD dwMask = ntohl(inet_addr(stNetworkInfo.stuInterfaces[0].szSubnetMask));
	m_ctlMask.SetAddress(dwMask);

	DWORD dwGateway = ntohl(inet_addr(stNetworkInfo.stuInterfaces[0].szDefGateway));
	m_ctlGateway.SetAddress(dwGateway);
#endif
}
