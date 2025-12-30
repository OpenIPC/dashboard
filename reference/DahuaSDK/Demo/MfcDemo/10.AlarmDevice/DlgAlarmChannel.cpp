// DlgAlarmChannel.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAlarmChannel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmChannel dialog


CDlgAlarmChannel::CDlgAlarmChannel(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgAlarmChannel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAlarmChannel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(NET_ALARM_CHANNELS));
	m_stuInfo.dwSize = sizeof(NET_ALARM_CHANNELS);
}


void CDlgAlarmChannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarmChannel)
	DDX_Control(pDX, IDC_ALARMCHANNELS_LIST_IN, m_lstAlarmInChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAlarmChannel, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarmChannel)
	ON_BN_CLICKED(IDC_ALARMCHANNELS_BTN_QUERY, OnAlarmchannelsBtnQuery)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmChannel private method

void CDlgAlarmChannel::InitDlg()
{
	GetDlgItem(IDC_ALARMCHANNELS_EDT_INRETCOUNT)->EnableWindow(FALSE);
	SetDlgItemInt(IDC_ALARMCHANNELS_EDT_INCOUNT, 64, FALSE);
	m_lstAlarmInChannel.SetExtendedStyle(m_lstAlarmInChannel.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_lstAlarmInChannel.InsertColumn(0, ConvertString("Index", DLG_ALARM_CHANNELS), LVCFMT_LEFT, 60, -1);
	m_lstAlarmInChannel.InsertColumn(1, ConvertString("Slot", DLG_ALARM_CHANNELS), LVCFMT_LEFT, 90, -1);
	m_lstAlarmInChannel.InsertColumn(2, ConvertString("Channel", DLG_ALARM_CHANNELS), LVCFMT_LEFT, 120, -1);
	m_lstAlarmInChannel.InsertColumn(3, ConvertString("Name", DLG_ALARM_CHANNELS), LVCFMT_LEFT, 150, -1);
}

BOOL CDlgAlarmChannel::getInfoFromDevice()
{
	int nMaxCount = GetDlgItemInt(IDC_ALARMCHANNELS_EDT_INCOUNT);
	if (nMaxCount <= 0)
	{
		MessageBox(ConvertString("Error param", DLG_ALARM_CHANNELS), ConvertString("Prompt"));
		return FALSE;
	}
	if (m_stuInfo.pstuChannelInfo != NULL)
	{
		delete[] m_stuInfo.pstuChannelInfo;
		m_stuInfo.pstuChannelInfo = NULL;
	}
	m_stuInfo.pstuChannelInfo = new NET_ALARM_CHANNEL_INFO[nMaxCount];
	if (NULL == m_stuInfo.pstuChannelInfo)
	{
		MessageBox(ConvertString("new error", DLG_ALARM_CHANNELS), ConvertString("Prompt"));
		return FALSE;
	}
	memset(m_stuInfo.pstuChannelInfo, 0, sizeof(NET_ALARM_CHANNEL_INFO) * nMaxCount);
	for (int i = 0; i < nMaxCount; i++)
	{
		m_stuInfo.pstuChannelInfo[i].dwSize = sizeof(NET_ALARM_CHANNEL_INFO);
	}
	m_stuInfo.nMaxAlarmChannels = nMaxCount;

	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_ALARM_CHANNELS, 
		(char*)&m_stuInfo, sizeof(NET_ALARM_CHANNELS), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query AlarmChannel failed:", DLG_ALARM_CHANNELS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		return TRUE;
	}
}

void CDlgAlarmChannel::showInfo()
{
	SetDlgItemInt(IDC_ALARMCHANNELS_EDT_INRETCOUNT, m_stuInfo.nRetAlarmChannels, FALSE);

	int nIndex		= 0;
	int nMinCount	= __min(m_stuInfo.nMaxAlarmChannels, m_stuInfo.nRetAlarmChannels);
	m_lstAlarmInChannel.DeleteAllItems();
	m_lstAlarmInChannel.DeleteAllItems();

	for (int i = 0; i < nMinCount; ++i)
	{
		char szIndex[10] = {0};
		char szSlot[10] = {0};
		char szChannel[10] = {0};
		_itoa(i + 1, szIndex, 10);
		_itoa(m_stuInfo.pstuChannelInfo[i].nSlot, szSlot, 10);
		_itoa(m_stuInfo.pstuChannelInfo[i].nChannel, szChannel, 10);
		m_lstAlarmInChannel.InsertItem(nIndex, NULL);
		m_lstAlarmInChannel.SetItemText(nIndex, 0, szIndex);
		m_lstAlarmInChannel.SetItemText(nIndex, 1, szSlot);
		m_lstAlarmInChannel.SetItemText(nIndex, 2, szChannel);
		m_lstAlarmInChannel.SetItemText(nIndex, 3, m_stuInfo.pstuChannelInfo[i].szName);
		nIndex++;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmChannel message handlers

BOOL CDlgAlarmChannel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARM_CHANNELS);
	// TODO: Add extra initialization here
	InitDlg();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAlarmChannel::OnAlarmchannelsBtnQuery() 
{
	// TODO: Add your control notification handler code here
	if (getInfoFromDevice())
	{
		showInfo();
	}
}

void CDlgAlarmChannel::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (NULL != m_stuInfo.pstuChannelInfo)
	{
		delete[] m_stuInfo.pstuChannelInfo;
		m_stuInfo.pstuChannelInfo = NULL;
	}
}
