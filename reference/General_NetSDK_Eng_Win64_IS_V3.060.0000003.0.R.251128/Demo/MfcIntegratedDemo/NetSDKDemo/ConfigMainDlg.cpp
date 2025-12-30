// ConfigMainDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigMainDlg dialog


CConfigMainDlg::CConfigMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigMainDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigMainDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDev = 0;
}


void CConfigMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigMainDlg)
	DDX_Control(pDX, IDC_TAB_BOARD, m_configBoard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigMainDlg, CDialog)
	//{{AFX_MSG_MAP(CConfigMainDlg)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_BOARD, OnSelchangeTabBoard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigMainDlg message handlers

BOOL CConfigMainDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	if (!m_pDev)
	{
		return TRUE;
	}
	
	m_configBoard.InsertItem(0, ConvertString(NAME_CFG_SERVERCONFIG));
	m_configBoard.InsertItem(1, ConvertString(NAME_CFG_CHANNELCONFIG));
	m_configBoard.InsertItem(2, ConvertString(NAME_CFG_SERIALCONFIG));
	m_configBoard.InsertItem(3, ConvertString(NAME_CFG_ALARMRECORD));
	m_configBoard.InsertItem(4, ConvertString(NAME_CFG_NETWORK));
	m_configBoard.InsertItem(5, ConvertString(NAME_CFG_ALARMCONFIG));

	m_serverDlg.SetDevice(m_pDev);
	m_channelDlg.SetDevice(m_pDev);
	m_commDlg.SetDevice(m_pDev);
	m_alarmDlg.SetDevice(m_pDev);
	m_recordDlg.SetDevice(m_pDev);
	m_networkDlg.SetDevice(m_pDev);

	CRect childRect;
	m_configBoard.GetClientRect(childRect);
	childRect.top += 30;
	childRect.bottom -= 4;
	childRect.left += 4;
	childRect.right -= 4;

	m_serverDlg.Create(IDD_CONFIG_SERVER, &m_configBoard);
	m_serverDlg.MoveWindow(childRect);
	m_channelDlg.Create(IDD_CONFIG_CHANNEL, &m_configBoard);
	m_channelDlg.MoveWindow(childRect);
	m_commDlg.Create(IDD_CONFIG_COMM, &m_configBoard);
	m_commDlg.MoveWindow(childRect);
	m_alarmDlg.Create(IDD_CONFIG_ALARM, &m_configBoard);
	m_alarmDlg.MoveWindow(childRect);
	m_recordDlg.Create(IDD_CONFIG_RECORD, &m_configBoard);
	m_recordDlg.MoveWindow(childRect);
	m_networkDlg.Create(IDD_CONFIG_NETWORK, &m_configBoard);
	m_networkDlg.MoveWindow(childRect);
	
	m_configBoard.SetCurSel(0);
	m_serverDlg.ShowWindow(SW_SHOW);
	m_channelDlg.ShowWindow(SW_HIDE);
	m_commDlg.ShowWindow(SW_HIDE);
	m_alarmDlg.ShowWindow(SW_HIDE);
	m_recordDlg.ShowWindow(SW_HIDE);
	m_networkDlg.ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigMainDlg::OnSelchangeTabBoard(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	switch(m_configBoard.GetCurSel())
	{
	case 0:			//server config board
		m_serverDlg.ShowWindow(SW_SHOW);
		m_channelDlg.ShowWindow(SW_HIDE);
		m_commDlg.ShowWindow(SW_HIDE);
		m_alarmDlg.ShowWindow(SW_HIDE);
		m_recordDlg.ShowWindow(SW_HIDE);
		m_networkDlg.ShowWindow(SW_HIDE);
		break;
	case 1:			//channel config board
		m_channelDlg.ShowWindow(SW_SHOW);
		m_serverDlg.ShowWindow(SW_HIDE);
		m_commDlg.ShowWindow(SW_HIDE);
		m_alarmDlg.ShowWindow(SW_HIDE);
		m_recordDlg.ShowWindow(SW_HIDE);
		m_networkDlg.ShowWindow(SW_HIDE);
		break;
	case 2:			//serial comm config board
		m_commDlg.ShowWindow(SW_SHOW);
		m_serverDlg.ShowWindow(SW_HIDE);
		m_channelDlg.ShowWindow(SW_HIDE);
		m_alarmDlg.ShowWindow(SW_HIDE);
		m_recordDlg.ShowWindow(SW_HIDE);
		m_networkDlg.ShowWindow(SW_HIDE);
		break;
	case 3:			//record config board	
		m_recordDlg.ShowWindow(SW_SHOW);
		m_alarmDlg.ShowWindow(SW_HIDE);
		m_serverDlg.ShowWindow(SW_HIDE);
		m_channelDlg.ShowWindow(SW_HIDE);
		m_commDlg.ShowWindow(SW_HIDE);
		m_networkDlg.ShowWindow(SW_HIDE);
		break;
	case 4:			//alarm config board
		m_networkDlg.ShowWindow(SW_SHOW);
		m_alarmDlg.ShowWindow(SW_HIDE);
		m_serverDlg.ShowWindow(SW_HIDE);
		m_channelDlg.ShowWindow(SW_HIDE);
		m_commDlg.ShowWindow(SW_HIDE);
		m_recordDlg.ShowWindow(SW_HIDE);
		break;
	case 5:
		m_alarmDlg.ShowWindow(SW_SHOW);
		m_networkDlg.ShowWindow(SW_HIDE);
		m_serverDlg.ShowWindow(SW_HIDE);
		m_channelDlg.ShowWindow(SW_HIDE);
		m_commDlg.ShowWindow(SW_HIDE);
		m_recordDlg.ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}
	*pResult = 0;
}

void CConfigMainDlg::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}
