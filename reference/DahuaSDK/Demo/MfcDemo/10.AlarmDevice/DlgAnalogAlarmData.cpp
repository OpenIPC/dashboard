// DlgAnalogAlarmData.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAnalogAlarmData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAnalogAlarmData dialog


CDlgAnalogAlarmData::CDlgAnalogAlarmData(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgAnalogAlarmData::IDD, pParent)
	, m_lLoginID(lLoginID)
	, m_lAttachID(0)
	, m_nIndex(0)
{
	//{{AFX_DATA_INIT(CDlgAnalogAlarmData)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgAnalogAlarmData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAnalogAlarmData)
	DDX_Control(pDX, IDC_ANALOGALARM_DATA_LIST_INFO, m_lstDataInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAnalogAlarmData, CDialog)
	//{{AFX_MSG_MAP(CDlgAnalogAlarmData)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ANALOGALARM_DATA_BTN_ATTACH, OnAnalogalarmDataBtnAttach)
	ON_BN_CLICKED(IDC_ANALOGALARM_DATA_BTN_DETACH, OnAnalogalarmDataBtnDetach)
	ON_MESSAGE(WM_ON_ANALOGALARM_DATA, OnAnalogAlarmData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Private method handlers

void CDlgAnalogAlarmData::GetTimeStringByStruct(NET_TIME stuTime, char *pTime)
{
	if (pTime)
	{
		sprintf(pTime, "%lu-%lu-%lu %lu:%lu:%lu", stuTime.dwYear, stuTime.dwMonth, stuTime.dwDay,
			stuTime.dwHour, stuTime.dwMinute, stuTime.dwSecond);
	}
}

void CDlgAnalogAlarmData::InitListControl()
{	
	m_lstDataInfo.SetExtendedStyle(m_lstDataInfo.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_lstDataInfo.InsertColumn(0, ConvertString("Index", DLG_ANALOGALARM_DATA), LVCFMT_LEFT, 40, -1);
	m_lstDataInfo.InsertColumn(1, ConvertString("Channel", DLG_ANALOGALARM_DATA), LVCFMT_LEFT, 50, -1);
	m_lstDataInfo.InsertColumn(2, ConvertString("Sensor", DLG_ANALOGALARM_DATA), LVCFMT_LEFT, 70, -1);
	m_lstDataInfo.InsertColumn(3, ConvertString("Data", DLG_ANALOGALARM_DATA), LVCFMT_LEFT, 80, -1);
	m_lstDataInfo.InsertColumn(4, ConvertString("Time", DLG_ANALOGALARM_DATA), LVCFMT_LEFT, 120, -1);
	m_lstDataInfo.InsertColumn(5, ConvertString("Status", DLG_ANALOGALARM_DATA), LVCFMT_LEFT, 60, -1);
}

void CDlgAnalogAlarmData::UpdateList()
{
	if (m_nIndex >= MAX_INFO_SHOW)
	{
		m_lstDataInfo.DeleteAllItems();
		m_nIndex = 0;
	}
}

void CDlgAnalogAlarmData::InsertEventInfo(const NET_ANALOGALARM_SENSE_INFO& stuInfo)
{
	UpdateList();
				
	char szIndex[32] = {0};
	_itoa(m_nIndex + 1, szIndex, 10);
	m_lstDataInfo.InsertItem(m_nIndex, NULL);
	
	// index
	m_lstDataInfo.SetItemText(m_nIndex, 0, szIndex);
	
	// channel id
	CString csChannelId;
	csChannelId.Format("%s %03d", ConvertString("Channel", DLG_ANALOGALARM_DATA), stuInfo.nChannelID + 1);
	m_lstDataInfo.SetItemText(m_nIndex, 1, csChannelId);

	// sensor
	CString csSensor;
	SenseTypeToStr(stuInfo.emSense, csSensor);
	m_lstDataInfo.SetItemText(m_nIndex, 2, ConvertString(csSensor, DLG_CFG_ALARM));

	// data 
	CString csData;
	csData.Format("%f", stuInfo.fData);
	m_lstDataInfo.SetItemText(m_nIndex, 3, csData);

	// time
	char szTime[64] = {0};
	GetTimeStringByStruct(stuInfo.stuTime, szTime);
	m_lstDataInfo.SetItemText(m_nIndex, 4, szTime);

	// status
	CString csStatus = AnalogAlarmDataStatusToStr(stuInfo.nStatus);
	m_lstDataInfo.SetItemText(m_nIndex, 5, ConvertString(csStatus, DLG_SUBSCRIBE));

	// update index
	m_nIndex++;
}

void CALLBACK OnAnalogAlarmDataCallBack(LLONG lLoginID, LLONG lAttachHandle, NET_ANALOGALARM_SENSE_INFO* pInfo, int nBufLen, LDWORD dwUser)
{
	((CDlgAnalogAlarmData*)dwUser)->AnalogAlarmDataCallBack(lLoginID, lAttachHandle, pInfo, nBufLen);
}

void CDlgAnalogAlarmData::AnalogAlarmDataCallBack(LLONG lLoginID, LLONG lAttachHandle, NET_ANALOGALARM_SENSE_INFO* pInfo, int nBufLen)
{
	if (lLoginID != m_lLoginID || lAttachHandle != m_lAttachID || NULL == pInfo || 0 == pInfo->dwSize || nBufLen <= 0)
	{
		return;
	}

	NET_ANALOGALARM_SENSE_INFO* pDemo = new NET_ANALOGALARM_SENSE_INFO;
	assert(pDemo != NULL);
	memset(pDemo, 0, sizeof(NET_ANALOGALARM_SENSE_INFO));
	pDemo->dwSize = sizeof(NET_ANALOGALARM_SENSE_INFO);

	int nSize = sizeof(pDemo->dwSize);
	nSize += sizeof(pDemo->nChannelID);
	if (nSize <= pInfo->dwSize)
	{
		pDemo->nChannelID = pInfo->nChannelID;
	}

	nSize += sizeof(pDemo->emSense);
	if (nSize <= pInfo->dwSize)
	{
		pDemo->emSense = pInfo->emSense;
	}

	nSize += sizeof(pDemo->fData);
	if (nSize <= pInfo->dwSize)
	{
		pDemo->fData = pInfo->fData;
	}

	nSize += sizeof(pDemo->stuTime);
	if (nSize <= pInfo->dwSize)
	{
		memcpy(&pDemo->stuTime, &pInfo->stuTime, sizeof(NET_TIME));
	}

    nSize += sizeof(pDemo->nStatus);
    if (nSize <= pInfo->dwSize)
    {
        pDemo->nStatus = pInfo->nStatus;
    }

	PostMessage(WM_ON_ANALOGALARM_DATA, (WPARAM)pDemo, (LPARAM)(pDemo->dwSize));
}

LRESULT CDlgAnalogAlarmData::OnAnalogAlarmData(WPARAM wParam, LPARAM lParam)
{
	NET_ANALOGALARM_SENSE_INFO* pInfo = (NET_ANALOGALARM_SENSE_INFO*)wParam;
	DWORD dwSize = (DWORD)lParam;
	if (NULL == pInfo || pInfo->dwSize != dwSize)
	{
		return -1;
	}
	
	InsertEventInfo(*pInfo);
	
	delete pInfo;
	pInfo = NULL;
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAnalogAlarmData message handlers

BOOL CDlgAnalogAlarmData::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ANALOGALARM_DATA);
	
	// TODO: Add extra initialization here
	InitListControl();
	GetDlgItem(IDC_ANALOGALARM_DATA_BTN_ATTACH)->EnableWindow();
	GetDlgItem(IDC_ANALOGALARM_DATA_BTN_DETACH)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAnalogAlarmData::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_lAttachID != NULL)
	{
		CLIENT_DetachAnalogAlarmData(m_lAttachID);
		m_lAttachID = NULL;
	}
}

void CDlgAnalogAlarmData::OnAnalogalarmDataBtnAttach() 
{
	// TODO: Add your control notification handler code here
	NET_IN_ANALOGALARM_DATA stuIn = {sizeof(stuIn)};
	NET_OUT_ANALOGALARM_DATA stuOut = {sizeof(stuOut)};

	stuIn.cbCallBack = OnAnalogAlarmDataCallBack;
	stuIn.dwUser = (LDWORD)this;
	stuIn.nChannelId = -1;

	LLONG lAttach = CLIENT_AttachAnalogAlarmData(m_lLoginID, &stuIn, &stuOut, SDK_API_WAIT);
	if (lAttach != NULL)
	{
		m_lAttachID = lAttach;
		m_lstDataInfo.DeleteAllItems();
		m_nIndex = 0;
		GetDlgItem(IDC_ANALOGALARM_DATA_BTN_ATTACH)->EnableWindow(FALSE);
		GetDlgItem(IDC_ANALOGALARM_DATA_BTN_DETACH)->EnableWindow();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s:0x%08x", ConvertString("Attach analog alarm data failed", DLG_ANALOGALARM_DATA), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("AnalogAlarmData", DLG_ANALOGALARM_DATA));
	}
}

void CDlgAnalogAlarmData::OnAnalogalarmDataBtnDetach() 
{
	// TODO: Add your control notification handler code here
	CLIENT_DetachAnalogAlarmData(m_lAttachID);
	m_lAttachID = NULL;
	GetDlgItem(IDC_ANALOGALARM_DATA_BTN_ATTACH)->EnableWindow();
	GetDlgItem(IDC_ANALOGALARM_DATA_BTN_DETACH)->EnableWindow(FALSE);
}
