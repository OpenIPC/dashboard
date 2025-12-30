// DlgExAlarmChannel.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgExAlarmChannel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmChannel dialog


CDlgExAlarmChannel::CDlgExAlarmChannel(CWnd* pParent /*=NULL*/, LLONG lLoginId /*=NULL*/)
	: CDialog(CDlgExAlarmChannel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgExAlarmChannel)
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0 , sizeof(NET_EXALARMCHANNELS));
	m_stuInfo.dwSize = sizeof(NET_EXALARMCHANNELS);
}


void CDlgExAlarmChannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgExAlarmChannel)
	DDX_Control(pDX, IDC_EXALARMCHANNEL_LIST_OUTREPORT, m_strExAlarmChannelOutList);
	DDX_Control(pDX, IDC_EXALARMCHANNEL_LIST_INREPORT, m_strExAlarmChannelInList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgExAlarmChannel, CDialog)
	//{{AFX_MSG_MAP(CDlgExAlarmChannel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EXALARMCHANNELS_BTN_GET, OnExalarmchannelsBtnGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmChannel message handlers

BOOL CDlgExAlarmChannel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_EXALARMCHANNEL);
	// TODO: Add extra initialization here
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_EXALARMCHANNEL), ConvertString("Prompt"));
		//EndDialog(0);
	}
	
	InitDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExAlarmChannel::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_stuInfo.pstuExAlarmInInfo != NULL)
	{
		delete []m_stuInfo.pstuExAlarmInInfo;
		m_stuInfo.pstuExAlarmInInfo = NULL;
	}

	if (m_stuInfo.pstuExAlarmOutInfo != NULL)
	{
		delete []m_stuInfo.pstuExAlarmOutInfo;
		m_stuInfo.pstuExAlarmOutInfo = NULL;
	}
}


void CDlgExAlarmChannel::OnExalarmchannelsBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (!getInfo())
	{
		MessageBox(ConvertString("new error!!!", DLG_EXALARMCHANNEL), ConvertString("Prompt"));
		return;
	}
	if (getInfoFromDevice())
	{
		showInfo();
	}
}

void CDlgExAlarmChannel::showInfo()
{
	SetDlgItemInt(IDC_EXALARMCHANNEL_EDIT_INRETCOUNT, m_stuInfo.nRetExAlarmInCount, FALSE);
	SetDlgItemInt(IDC_EXALARMCHANNEL_EDIT_OUTRETCOUNT, m_stuInfo.nRetExAlarmOutCount, FALSE);
	int i = 0, nExAlarmRptIndex = 0;
	int nMinCount = __min(m_stuInfo.nExAlarmInCount, m_stuInfo.nRetExAlarmInCount);
	m_strExAlarmChannelInList.DeleteAllItems();
	m_strExAlarmChannelOutList.DeleteAllItems();
	for (i = 0; i < nMinCount; ++i)
	{
		char szIndex[10] = {0};
		char szExAlarmBoxNum[10] = {0};
		char szExAlarmChannelNum[10] = {0};
		_itoa(i + 1, szIndex, 10);
		_itoa(m_stuInfo.pstuExAlarmInInfo[i].nExAlarmBoxNum, szExAlarmBoxNum, 10);
		_itoa(m_stuInfo.pstuExAlarmInInfo[i].nChannelNum, szExAlarmChannelNum, 10);
		m_strExAlarmChannelInList.InsertItem(nExAlarmRptIndex, NULL);
		m_strExAlarmChannelInList.SetItemText(nExAlarmRptIndex, 0, szIndex);
		m_strExAlarmChannelInList.SetItemText(nExAlarmRptIndex, 1, szExAlarmBoxNum);
		m_strExAlarmChannelInList.SetItemText(nExAlarmRptIndex, 2, szExAlarmChannelNum);
		m_strExAlarmChannelInList.SetItemText(nExAlarmRptIndex, 3, m_stuInfo.pstuExAlarmInInfo[i].szChannelName);
		nExAlarmRptIndex++;
	}

	nExAlarmRptIndex = 0;
	nMinCount = __min(m_stuInfo.nExAlarmOutCount, m_stuInfo.nRetExAlarmOutCount);
	for (i = 0; i < nMinCount; ++i)
	{
		char szIndex[10] = {0};
		m_strExAlarmChannelOutList.SetItemText(nExAlarmRptIndex, 0, szIndex);
		char szExAlarmBoxNum[10] = {0};
		char szExAlarmChannelNum[10] = {0};
		_itoa(i + 1, szIndex, 10);
		_itoa(m_stuInfo.pstuExAlarmOutInfo[i].nExAlarmBoxNum, szExAlarmBoxNum, 10);
		_itoa(m_stuInfo.pstuExAlarmOutInfo[i].nChannelNum, szExAlarmChannelNum, 10);
		m_strExAlarmChannelOutList.InsertItem(nExAlarmRptIndex, NULL);
		m_strExAlarmChannelOutList.SetItemText(nExAlarmRptIndex, 0, szIndex);
		m_strExAlarmChannelOutList.SetItemText(nExAlarmRptIndex, 1, szExAlarmBoxNum);
		m_strExAlarmChannelOutList.SetItemText(nExAlarmRptIndex, 2, szExAlarmChannelNum);
		m_strExAlarmChannelOutList.SetItemText(nExAlarmRptIndex, 3, m_stuInfo.pstuExAlarmOutInfo[i].szChannelName);
		nExAlarmRptIndex++;
	}
}

BOOL CDlgExAlarmChannel::getInfo()
{
	m_stuInfo.nExAlarmInCount = GetDlgItemInt(IDC_EXALARMCHANNEL_EDIT_INCOUNT);
	m_stuInfo.nExAlarmOutCount = GetDlgItemInt(IDC_EXALARMCHANNEL_EDIT_OUTCOUNT);
	if (m_stuInfo.nExAlarmInCount > 0)
	{
		if (m_stuInfo.pstuExAlarmInInfo != NULL)
		{
			delete []m_stuInfo.pstuExAlarmInInfo;
			m_stuInfo.pstuExAlarmInInfo = NULL;
		}
		m_stuInfo.pstuExAlarmInInfo = new NET_EXALARMCHANNELS_INFO[m_stuInfo.nExAlarmInCount];
		if (m_stuInfo.pstuExAlarmInInfo == NULL)
		{
			return FALSE;
		}
		memset(m_stuInfo.pstuExAlarmInInfo, 0, sizeof(NET_EXALARMCHANNELS_INFO) * m_stuInfo.nExAlarmInCount);
	}

	if (m_stuInfo.nExAlarmOutCount > 0)
	{
		if (m_stuInfo.pstuExAlarmOutInfo != NULL)
		{
			delete []m_stuInfo.pstuExAlarmOutInfo;
			m_stuInfo.pstuExAlarmOutInfo = NULL;
		}
		m_stuInfo.pstuExAlarmOutInfo = new NET_EXALARMCHANNELS_INFO[m_stuInfo.nExAlarmOutCount];
		if (m_stuInfo.pstuExAlarmOutInfo == NULL)
		{
			if (m_stuInfo.pstuExAlarmInInfo != NULL)
			{
				delete []m_stuInfo.pstuExAlarmInInfo;
				m_stuInfo.pstuExAlarmInInfo = NULL;
			}
			return FALSE;
		}
		memset(m_stuInfo.pstuExAlarmOutInfo, 0, sizeof(NET_EXALARMCHANNELS_INFO) * m_stuInfo.nExAlarmOutCount);
	}
	return TRUE;
}


BOOL CDlgExAlarmChannel::getInfoFromDevice()
{
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_EXALARMCHANNELS, 
		(char*)&m_stuInfo, sizeof(NET_EXALARMCHANNELS), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryExAlarmChannel failed:", DLG_EXALARMCHANNEL), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
// 		CString csErr;
// 		csErr.Format("QueryExAlarmChannel ok!");
// 		MessageBox(csErr, ConvertString("Prompt"));
		return TRUE;
	}
}

void CDlgExAlarmChannel::InitDlg()
{
	GetDlgItem(IDC_EXALARMCHANNEL_EDIT_INRETCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EXALARMCHANNEL_EDIT_OUTRETCOUNT)->EnableWindow(FALSE);
	SetDlgItemInt(IDC_EXALARMCHANNEL_EDIT_INCOUNT, 64, FALSE);
	SetDlgItemInt(IDC_EXALARMCHANNEL_EDIT_OUTCOUNT, 64, FALSE);
	m_strExAlarmChannelInList.SetExtendedStyle(m_strExAlarmChannelInList.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_strExAlarmChannelInList.InsertColumn(0, ConvertString("Index", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 60, -1);
	m_strExAlarmChannelInList.InsertColumn(1, ConvertString("ExAlarmBox", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 90, -1);
	m_strExAlarmChannelInList.InsertColumn(2, ConvertString("InChannel", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 120, -1);
	m_strExAlarmChannelInList.InsertColumn(3, ConvertString("Name", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 150, -1);

	m_strExAlarmChannelOutList.SetExtendedStyle(m_strExAlarmChannelInList.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_strExAlarmChannelOutList.InsertColumn(0, ConvertString("Index", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 60, -1);
	m_strExAlarmChannelOutList.InsertColumn(1, ConvertString("ExAlarmBox", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 90, -1);
	m_strExAlarmChannelOutList.InsertColumn(2, ConvertString("OutChannel", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 120, -1);
	m_strExAlarmChannelOutList.InsertColumn(3, ConvertString("Name", DLG_EXALARMCHANNEL), LVCFMT_LEFT, 150, -1);
}

