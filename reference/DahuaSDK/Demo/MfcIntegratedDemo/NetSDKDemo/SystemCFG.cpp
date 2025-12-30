// SystemCFG.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "SystemCFG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemCFG dialog


CSystemCFG::CSystemCFG(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemCFG::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSystemCFG)
	//}}AFX_DATA_INIT
}


void CSystemCFG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemCFG)
	DDX_Control(pDX, IDC_CONFIG_TAB, m_config_tab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSystemCFG, CDialog)
	//{{AFX_MSG_MAP(CSystemCFG)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CONFIG_TAB, OnSelchangeConfigTab)
	ON_BN_CLICKED(IDC_BTNQUIT, OnBtnquit)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemCFG message handlers

BOOL CSystemCFG::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	m_config_tab.InsertItem(0, ConvertString(NAME_CFG_SERVERCONFIG));
	m_ServerConfig.SetLoginID(m_Device->LoginID);
	m_ServerConfig.Create(IDD_CHILD_SERVERCFG, &m_config_tab);

	CRect rect;
	m_config_tab.GetClientRect(rect);
	rect.top += 30;
	rect.bottom -= 4;
	rect.left += 4;
	rect.right -= 4;
	m_ServerConfig.MoveWindow(rect);
	
	m_config_tab.InsertItem(1, ConvertString(NAME_CFG_CHANNELCONFIG));
	m_ChannelConfig.SetDevice(m_Device);
	m_ChannelConfig.Create(IDD_CHILD_CHANNELCFG, &m_config_tab);
	m_ChannelConfig.MoveWindow(rect);

	m_config_tab.InsertItem(2, ConvertString(NAME_CFG_SERIALCONFIG));
	m_SerialConfig.SetDevice(m_Device);
	m_SerialConfig.Create(IDD_CHILD_SERIALCFG, &m_config_tab);
	m_SerialConfig.MoveWindow(rect);

	m_config_tab.InsertItem(3, ConvertString(NAME_CFG_ALARMCONFIG));
	m_AlarmConfig.SetDevice(m_Device);
	m_AlarmConfig.Create(IDD_CHILD_ALARMCFG, &m_config_tab);
	m_AlarmConfig.MoveWindow(rect);

	m_config_tab.InsertItem(4, ConvertString(NAME_CFG_USERCONFIG));
	m_UserConfig.SetDevice(m_Device);
	m_UserConfig.Create(IDD_CHILD_USERCFG, &m_config_tab);
	m_UserConfig.MoveWindow(rect);

	//ATM»ú
	m_config_tab.InsertItem(5, NAME_CFG_TRADECONFIG);
	m_ATMConfig.Create(IDD_CHILD_ATMCFG, &m_config_tab);
	m_ATMConfig.MoveWindow(rect);

	m_config_tab.InsertItem(6, NAME_CFG_EXCEPTIONCONFIG);
	m_ExceptionConfig.Create(IDD_CHILD_EXCEPTIONCFG, &m_config_tab);
	m_ExceptionConfig.MoveWindow(rect);

	m_ServerConfig.ShowWindow(SW_SHOW);
	m_ChannelConfig.ShowWindow(SW_HIDE);
	m_SerialConfig.ShowWindow(SW_HIDE);
	m_AlarmConfig.ShowWindow(SW_HIDE);
	m_UserConfig.ShowWindow(SW_HIDE);
	m_ATMConfig.ShowWindow(SW_HIDE);
	m_ExceptionConfig.ShowWindow(SW_HIDE);

	return TRUE; 
}

void CSystemCFG::OnSelchangeConfigTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch(m_config_tab.GetCurSel())
	{
	case 0:
		m_ServerConfig.ShowWindow(SW_SHOW);
		m_ChannelConfig.ShowWindow(SW_HIDE);
		m_SerialConfig.ShowWindow(SW_HIDE);
		m_AlarmConfig.ShowWindow(SW_HIDE);
		m_UserConfig.ShowWindow(SW_HIDE);
		m_ATMConfig.ShowWindow(SW_HIDE);
		m_ExceptionConfig.ShowWindow(SW_HIDE);
		break;
	case 1:
		m_ServerConfig.ShowWindow(SW_HIDE);
		m_ChannelConfig.ShowWindow(SW_SHOW);
		m_SerialConfig.ShowWindow(SW_HIDE);
		m_AlarmConfig.ShowWindow(SW_HIDE);
		m_UserConfig.ShowWindow(SW_HIDE);
		m_ATMConfig.ShowWindow(SW_HIDE);
		m_ExceptionConfig.ShowWindow(SW_HIDE);
		break;
	case 2:
		m_ServerConfig.ShowWindow(SW_HIDE);
		m_ChannelConfig.ShowWindow(SW_HIDE);
		m_SerialConfig.ShowWindow(SW_SHOW);
		m_AlarmConfig.ShowWindow(SW_HIDE);
		m_UserConfig.ShowWindow(SW_HIDE);
		m_ATMConfig.ShowWindow(SW_HIDE);
		m_ExceptionConfig.ShowWindow(SW_HIDE);
		break;
	case 3:
		m_ServerConfig.ShowWindow(SW_HIDE);
		m_ChannelConfig.ShowWindow(SW_HIDE);
		m_SerialConfig.ShowWindow(SW_HIDE);
		m_AlarmConfig.ShowWindow(SW_SHOW);
		m_UserConfig.ShowWindow(SW_HIDE);
		m_ATMConfig.ShowWindow(SW_HIDE);
		m_ExceptionConfig.ShowWindow(SW_HIDE);
		break;
	case 4:
		m_ServerConfig.ShowWindow(SW_HIDE);
		m_ChannelConfig.ShowWindow(SW_HIDE);
		m_SerialConfig.ShowWindow(SW_HIDE);
		m_AlarmConfig.ShowWindow(SW_HIDE);
		m_UserConfig.ShowWindow(SW_SHOW);
		m_ATMConfig.ShowWindow(SW_HIDE);
		m_ExceptionConfig.ShowWindow(SW_HIDE);
		break;
	case 5:
		m_ServerConfig.ShowWindow(SW_HIDE);
		m_ChannelConfig.ShowWindow(SW_HIDE);
		m_SerialConfig.ShowWindow(SW_HIDE);
		m_AlarmConfig.ShowWindow(SW_HIDE);
		m_UserConfig.ShowWindow(SW_HIDE);
		m_ATMConfig.ShowWindow(SW_SHOW);
		m_ExceptionConfig.ShowWindow(SW_HIDE);
		break;
	case 6:
		m_ServerConfig.ShowWindow(SW_HIDE);
		m_ChannelConfig.ShowWindow(SW_HIDE);
		m_SerialConfig.ShowWindow(SW_HIDE);
		m_AlarmConfig.ShowWindow(SW_HIDE);
		m_UserConfig.ShowWindow(SW_HIDE);
		m_ATMConfig.ShowWindow(SW_HIDE);
		m_ExceptionConfig.ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
	*pResult = 0;
}

void CSystemCFG::SetDevice(DeviceNode *nDev)
{
	m_Device = nDev;
}

DeviceNode *CSystemCFG::GetDevice()
{
	return m_Device;
}

void CSystemCFG::OnBtnquit() 
{
	EndDialog(0);
}

void CSystemCFG::OnClose() 
{
	CDialog::OnClose();
}
