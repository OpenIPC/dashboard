// DlgAnalogAlarmChannels.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAnalogAlarmChannels.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAnalogAlarmChannels dialog


CDlgAnalogAlarmChannels::CDlgAnalogAlarmChannels(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgAnalogAlarmChannels::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAnalogAlarmChannels)
	m_edtMaxCount = 0;
	m_edtRetCount = 0;
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(NET_ANALOGALARM_CHANNELS));
	m_stuInfo.dwSize = sizeof(NET_ANALOGALARM_CHANNELS);
}


void CDlgAnalogAlarmChannels::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAnalogAlarmChannels)
	DDX_Control(pDX, IDC_ANALOGALARMCHANNELS_LIST_ANALOGCHANNELMAP, m_listAnalogChannelMap);
	DDX_Text(pDX, IDC_ANALOGALARMCHANNELS_EDIT_MAXCOUNT, m_edtMaxCount);
	DDX_Text(pDX, IDC_ANALOGALARMCHANNELS_EDIT_RETCOUNT, m_edtRetCount);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CDlgAnalogAlarmChannels, CDialog)
	//{{AFX_MSG_MAP(CDlgAnalogAlarmChannels)
	ON_BN_CLICKED(IDC_ANALOGALARMCHANNELS_BTN_GET, OnAnalogalarmchannelsBtnGet)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAnalogAlarmChannels message handlers

BOOL CDlgAnalogAlarmChannels::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ANALOGALARMCHANNELS);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_ANALOGALARMCHANNELS), ConvertString("Prompt"));
		return FALSE;
	}
	GetDlgItem(IDC_ANALOGALARMCHANNELS_EDIT_RETCOUNT)->EnableWindow(FALSE);
	//SetDlgItemInt(IDC_ANALOGALARMCHANNELS_EDIT_MAXCOUNT, 16, FALSE);
	m_listAnalogChannelMap.SetExtendedStyle(m_listAnalogChannelMap.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listAnalogChannelMap.InsertColumn(0, ConvertString("Index", DLG_ANALOGALARMCHANNELS), LVCFMT_LEFT, 60, -1);
	m_listAnalogChannelMap.InsertColumn(1, ConvertString("Slot", DLG_ANALOGALARMCHANNELS), LVCFMT_LEFT, 60, -1);
	m_listAnalogChannelMap.InsertColumn(2, ConvertString("Level1", DLG_ANALOGALARMCHANNELS), LVCFMT_LEFT, 60, -1);
	m_listAnalogChannelMap.InsertColumn(3, ConvertString("Level2", DLG_ANALOGALARMCHANNELS), LVCFMT_LEFT, 60, -1);
	m_listAnalogChannelMap.InsertColumn(4, ConvertString("Name", DLG_ANALOGALARMCHANNELS), LVCFMT_LEFT, 150, -1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAnalogAlarmChannels::OnAnalogalarmchannelsBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (!ApplyMemory())
	{
		MessageBox(ConvertString("new error!!!", DLG_ANALOGALARMCHANNELS), ConvertString("Prompt"));
		return;
	}
	if (getInfoFromDevice())
	{
		ShowInfo();
	}
}

BOOL CDlgAnalogAlarmChannels::getInfoFromDevice()
{
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_ANALOGALARM_CHANNELS, 
		(char*)&m_stuInfo, sizeof(NET_ANALOGALARM_CHANNELS), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query AnalogAlarmChannel failed:", DLG_ANALOGALARMCHANNELS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		return TRUE;
	}
}

BOOL CDlgAnalogAlarmChannels::ApplyMemory()
{
	m_stuInfo.nMaxAnalogAlarmChannels = GetDlgItemInt(IDC_ANALOGALARMCHANNELS_EDIT_MAXCOUNT);
	if (m_stuInfo.nMaxAnalogAlarmChannels > 0)
	{
		if (m_stuInfo.pstuChannelInfo != NULL)
		{
			delete []m_stuInfo.pstuChannelInfo;
			m_stuInfo.pstuChannelInfo = NULL;
		}
		m_stuInfo.pstuChannelInfo = new NET_ANALOGALARM_CHANNELS_INFO[m_stuInfo.nMaxAnalogAlarmChannels];
		if (m_stuInfo.pstuChannelInfo == NULL)
		{
			return FALSE;
		}
		memset(m_stuInfo.pstuChannelInfo, 0, sizeof(NET_ANALOGALARM_CHANNELS_INFO) * m_stuInfo.nMaxAnalogAlarmChannels);
		for (int i = 0; i < m_stuInfo.nMaxAnalogAlarmChannels; i++)
		{
			m_stuInfo.pstuChannelInfo[i].dwSize = sizeof(NET_ANALOGALARM_CHANNELS_INFO);
		}
	}
	
	return TRUE;
}

void CDlgAnalogAlarmChannels::ShowInfo()
{
	SetDlgItemInt(IDC_ANALOGALARMCHANNELS_EDIT_RETCOUNT, m_stuInfo.nRetAnalogAlarmChannels, FALSE);
	int i = 0, nExAlarmRptIndex = 0;
	int nMinCount = __min(m_stuInfo.nRetAnalogAlarmChannels, m_stuInfo.nMaxAnalogAlarmChannels);
	m_listAnalogChannelMap.DeleteAllItems();
	for (i = 0; i < nMinCount; ++i)
	{
		char szIndex[10] = {0};
		char szSlot[10] = {0};
		char szLevel1[10] = {0};
		char szLevel2[10] = {0};
		_itoa(i + 1, szIndex, 10);
		_itoa(m_stuInfo.pstuChannelInfo[i].nSlot, szSlot, 10);
		_itoa(m_stuInfo.pstuChannelInfo[i].nLevel1, szLevel1, 10);
		_itoa(m_stuInfo.pstuChannelInfo[i].nLevel2, szLevel2, 10);
		m_listAnalogChannelMap.InsertItem(nExAlarmRptIndex, NULL);
		m_listAnalogChannelMap.SetItemText(nExAlarmRptIndex, 0, szIndex);
		m_listAnalogChannelMap.SetItemText(nExAlarmRptIndex, 1, szSlot);
		m_listAnalogChannelMap.SetItemText(nExAlarmRptIndex, 2, szLevel1);
		m_listAnalogChannelMap.SetItemText(nExAlarmRptIndex, 3, szLevel2);
		m_listAnalogChannelMap.SetItemText(nExAlarmRptIndex, 4, m_stuInfo.pstuChannelInfo[i].szName);
		nExAlarmRptIndex++;
	}
}

void CDlgAnalogAlarmChannels::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_stuInfo.pstuChannelInfo != NULL)
	{
		delete []m_stuInfo.pstuChannelInfo;
		m_stuInfo.pstuChannelInfo = NULL;
	}

}
