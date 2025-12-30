// AlarmSubscribe.cpp : 实现文件
//

#include "stdafx.h"
#include "PassengerFlow.h"
#include "AlarmSubscribe.h"

#define WM_DEVICE_DOATTACH (WM_USER + 311)

#define SHOW_EVENT_PIC_SIZE	100

struct strAnalyInfo
{
	LLONG lAnalyzerHandle;
	DWORD dwAlarmType;
	void* pAlarmInfo;
	BYTE *pBuffer;
	DWORD dwBufSize;
};

int  GetAlarmInfoSize( DWORD AlarmType, DWORD *pAlarmSize)
{
	switch(AlarmType)
	{
	case EVENT_IVS_NUMBERSTAT:
		{
			*pAlarmSize = sizeof(DEV_EVENT_NUMBERSTAT_INFO);
		}
		break;
	case EVENT_IVS_MAN_NUM_DETECTION:
		{
			*pAlarmSize = sizeof(DEV_EVENT_MANNUM_DETECTION_INFO);
		}
		break;
	case EVENT_IVS_STAYDETECTION:
		{
			*pAlarmSize = sizeof(DEV_EVENT_STAY_INFO);
		}
		break;

	default:
		return -1; // demo not support this event type 
	}

	return 0;
}

int CALLBACK AnalyzerData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
	if(dwUser == 0)
	{
		return 0;
	}

	DWORD nAlarmInfoSize = 0;
	int nRet =  GetAlarmInfoSize(dwAlarmType, &nAlarmInfoSize);
	if (nRet < 0)
	{
		return -1;
	}

	CAlarmSubscribe *dlg = (CAlarmSubscribe*)dwUser;

	HWND hWnd = dlg->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return 0;
	}
	strAnalyInfo *info = new strAnalyInfo;
	memset(info, 0, sizeof(strAnalyInfo));
	info->lAnalyzerHandle = lAnalyzerHandle;
	info->dwAlarmType = dwAlarmType;

	info->pAlarmInfo = new char[nAlarmInfoSize];
	memcpy(info->pAlarmInfo, pAlarmInfo, nAlarmInfoSize);

	info->dwBufSize=dwBufSize;
	if (dwBufSize > 0)
	{
		info->pBuffer = new BYTE[dwBufSize];
		memset(info->pBuffer, 0, dwBufSize);
		memcpy(info->pBuffer, pBuffer, dwBufSize);
	}

	::PostMessage(hWnd, WM_DEVICE_DOATTACH, WPARAM(info), NULL);

	return 1;
}

// CAlarmSubscribe 对话框

IMPLEMENT_DYNAMIC(CAlarmSubscribe, CDialog)

CAlarmSubscribe::CAlarmSubscribe(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmSubscribe::IDD, pParent)
{
	m_lRealPicHandle = 0;
	m_lLoginId = 0;
	m_nChannel = 0;
	m_index = 0;
}

CAlarmSubscribe::~CAlarmSubscribe()
{
	DeleteAllStoreEventData();
	m_dlgShowPic.DestroyWindow();
}

void CAlarmSubscribe::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_REAL_ATTCHINFO, m_lcAlarmInfo);
}


BEGIN_MESSAGE_MAP(CAlarmSubscribe, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ALARMATTACH, &CAlarmSubscribe::OnBnClickedButtonAlarmattach)
	ON_BN_CLICKED(IDC_BUTTON_ALARMDETACH, &CAlarmSubscribe::OnBnClickedButtonAlarmdetach)
	ON_MESSAGE(WM_DEVICE_DOATTACH, &CAlarmSubscribe::DoDeviceAttach)

	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REAL_ATTCHINFO, &CAlarmSubscribe::OnLvnItemchangedListRealAttchinfo)
END_MESSAGE_MAP()


void CAlarmSubscribe::InitAlarmInfoLct()
{
	m_lcAlarmInfo.SetExtendedStyle(m_lcAlarmInfo.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lcAlarmInfo.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,50,-1);
	m_lcAlarmInfo.InsertColumn(1,ConvertString("Channel"),LVCFMT_LEFT,60,0);
	m_lcAlarmInfo.InsertColumn(2,ConvertString("AlarmType"),LVCFMT_LEFT,100,1);
	m_lcAlarmInfo.InsertColumn(3, ConvertString("RuleName"), LVCFMT_LEFT,100,2);
	m_lcAlarmInfo.InsertColumn(4,ConvertString("AlarmInfo"),LVCFMT_LEFT,400,3);
	m_lcAlarmInfo.InsertColumn(5, ConvertString("Time"), LVCFMT_LEFT,150,4);
}

BOOL CAlarmSubscribe::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	m_dlgShowPic.Create(IDD_PICTURE_DIALOG);

	InitAlarmInfoLct();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CAlarmSubscribe::InitALarmDlg(int nChannel, LLONG mLoginID)
{
	m_lLoginId = mLoginID;
	m_nChannel = 0;
	GetDlgItem(IDC_BUTTON_ALARMATTACH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ALARMDETACH)->EnableWindow(FALSE);
}

void CAlarmSubscribe::OnBnClickedButtonAlarmattach()
{
	m_index = 0;
	m_lRealPicHandle = CLIENT_RealLoadPictureEx(m_lLoginId, m_nChannel, EVENT_IVS_ALL, TRUE, AnalyzerData, (DWORD)this, NULL);
	if (0 == m_lRealPicHandle)
	{
		MessageBox(ConvertString("Failed to attch passenger flow alarm!"), "");
	}
	else
	{
		GetDlgItem(IDC_BUTTON_ALARMATTACH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ALARMDETACH)->EnableWindow(TRUE);
	}
}

void CAlarmSubscribe::SetChannelNum(int nChannel)
{
	m_nChannel = nChannel;
}

void CAlarmSubscribe::OnBnClickedButtonAlarmdetach()
{
	if (m_lRealPicHandle)
	{
		CLIENT_StopLoadPic(m_lRealPicHandle);
	}
	m_lRealPicHandle = 0;

	DeleteAllItemInfo();
	DeleteAllStoreEventData();

	GetDlgItem(IDC_BUTTON_ALARMATTACH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ALARMDETACH)->EnableWindow(FALSE);
}

void CAlarmSubscribe::ExitDlg()
{
	m_lcAlarmInfo.DeleteAllItems();
	OnBnClickedButtonAlarmdetach();
	GetDlgItem(IDC_BUTTON_ALARMATTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ALARMDETACH)->EnableWindow(FALSE);
}

LRESULT CAlarmSubscribe::DoDeviceAttach(WPARAM wParam, LPARAM lParam)
{
	strAnalyInfo *pInfo = (strAnalyInfo *)wParam;
	if (pInfo == NULL)
	{
		return 0;
	}

	DoAttchData(pInfo->lAnalyzerHandle, pInfo->dwAlarmType, pInfo->pAlarmInfo, pInfo->pBuffer, pInfo->dwBufSize);

	if (pInfo->pBuffer)
	{
		delete[] pInfo->pBuffer;
		pInfo->pBuffer = NULL;
	}
	if (pInfo->pAlarmInfo)
	{
		delete pInfo->pAlarmInfo;
		pInfo->pAlarmInfo = NULL;
	}

	if (pInfo)
	{
		delete pInfo;
		pInfo = NULL;
	}
	return 0;
}

void CAlarmSubscribe::DoAttchData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize)
{
	if (0 == lAnalyzerHandle || NULL == pAlarmInfo)
	{
		MessageBox(ConvertString("Invalid param!"), ConvertString("Prompt"));
		return;
	}

	int nChannel = -1;
	CString strType = "";
	CString strRuleName = "";
	CString strAlarmInfo = "";
	CString strTime = "";
	switch(dwAlarmType)
	{
	case EVENT_IVS_NUMBERSTAT: // 人数统计
		{
			DEV_EVENT_NUMBERSTAT_INFO *pstInfo = (DEV_EVENT_NUMBERSTAT_INFO*)pAlarmInfo;
			nChannel = pstInfo->nChannelID;
			strType = "NumberStat";
			strRuleName.Format("%s", pstInfo->szName);
			strAlarmInfo.Format("%s:%d  %s:%d  %s:%d",
							ConvertString("Enter").GetBuffer(),
							pstInfo->nEnteredNumber,
							ConvertString("Exit").GetBuffer(),
							pstInfo->nExitedNumber,
							ConvertString("Inside").GetBuffer(),
							pstInfo->nNumber);
							
			strTime.Format("%d-%02d-%02d %02d:%02d:%02d", pstInfo->stuFileInfo.stuFileTime.dwYear,
														pstInfo->stuFileInfo.stuFileTime.dwMonth,
														pstInfo->stuFileInfo.stuFileTime.dwDay,
														pstInfo->stuFileInfo.stuFileTime.dwHour,
														pstInfo->stuFileInfo.stuFileTime.dwMinute,
														pstInfo->stuFileInfo.stuFileTime.dwSecond);
			break;
		}
	case  EVENT_IVS_MAN_NUM_DETECTION:			//区域内人数统计
		{
			DEV_EVENT_MANNUM_DETECTION_INFO *pstInfo = (DEV_EVENT_MANNUM_DETECTION_INFO*)pAlarmInfo;

			nChannel = pstInfo->nChannelID;
			strType = "ManNumDetection";
			strRuleName.Format("%s", pstInfo->szName);
			strAlarmInfo.Format("%s:%d", 
							ConvertString("Current region Man Number").GetBuffer(),
							pstInfo->nManListCount);
			strTime.Format("%d-%02d-%02d %02d:%02d:%02d",pstInfo->UTC.dwYear,
														pstInfo->UTC.dwMonth,
														pstInfo->UTC.dwDay,
														pstInfo->UTC.dwHour,
														pstInfo->UTC.dwMinute,
														pstInfo->UTC.dwSecond);
			break;
		}
	case EVENT_IVS_STAYDETECTION:				//滞留报警
		{
			DEV_EVENT_STAY_INFO *pstInfo = (DEV_EVENT_STAY_INFO *)pAlarmInfo;
			nChannel = pstInfo->nChannelID;
			strType = "StayDetection";
			strRuleName.Format("%s", pstInfo->szName);

			strTime.Format("%d-%02d-%02d %02d:%02d:%02d",pstInfo->UTC.dwYear,
									pstInfo->UTC.dwMonth,
									pstInfo->UTC.dwDay,
									pstInfo->UTC.dwHour,
									pstInfo->UTC.dwMinute,
									pstInfo->UTC.dwSecond);
			LLONG lTimeBegin = pstInfo->stuObject.stuStartTime.dwYear;

			strAlarmInfo = "StayDetection";
			break;
		}
	default:
		return;
	}

	GUID guid = {0};
	HRESULT hr =  CoCreateGuid(&guid);
	char bufGUID[64] = {0};
	sprintf_s(
		bufGUID,
		"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	std::string strGuid(bufGUID);
	StoreEventPicInfo(pBuffer, dwBufSize, strGuid);
	DisplayEventInfo(nChannel, strType, strRuleName, strAlarmInfo, strTime, strGuid);
}

void CAlarmSubscribe::StoreEventPicInfo(BYTE *pBuffer, DWORD dwBufSize, std::string& strGuid)
{
	EventPicInfo *picInfo = new EventPicInfo;
	picInfo->pPicBuf = new BYTE[dwBufSize];
	memset(picInfo->pPicBuf, 0, dwBufSize);
	memcpy(picInfo->pPicBuf , pBuffer, dwBufSize);
	picInfo->dwBufSize = dwBufSize;
	picInfo->strGuid = strGuid;
	m_EventPicList.push_front(picInfo);
	
	if (m_EventPicList.size() > SHOW_EVENT_PIC_SIZE)
	{
		std::list<EventPicInfo*>::reverse_iterator it= m_EventPicList.rbegin();
		EventPicInfo *pInfo = (*it);
		if (pInfo)
		{
			delete[] pInfo->pPicBuf;
			pInfo->pPicBuf = NULL;
			delete pInfo;
			pInfo = NULL;
		}
		m_EventPicList.pop_back();
	}
}

void CAlarmSubscribe::DisplayEventInfo(int nChannel, CString strType, CString strRuleName, CString strAlarmInfo, CString strTime, std::string &strGuid)
{
	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = _T("");
	lvi.iImage = 0;
	lvi.iItem = 0;

	m_lcAlarmInfo.InsertItem(&lvi);

	m_index++;
	CString strIndex;
	strIndex.Format("%d", m_index);
	CString strChannel;
	strChannel.Format("%d", nChannel + 1);
	m_lcAlarmInfo.SetItemText(0,0,strIndex);
	m_lcAlarmInfo.SetItemText(0,1,ConvertString(strChannel));
	m_lcAlarmInfo.SetItemText(0,2,ConvertString(strType));
	m_lcAlarmInfo.SetItemText(0,3, ConvertString(strRuleName));
	m_lcAlarmInfo.SetItemText(0,4, ConvertString(strAlarmInfo));
	m_lcAlarmInfo.SetItemText(0,5, ConvertString(strTime));

	char *pGuid = new char[64];
	memset(pGuid, 0, 64);
	strncpy(pGuid, strGuid.c_str(), 63);
	m_lcAlarmInfo.SetItemData(0, (DWORD_PTR)pGuid);

	if (m_lcAlarmInfo.GetItemCount() > SHOW_EVENT_PIC_SIZE)
	{
		char* pCh = (char*) m_lcAlarmInfo.GetItemData(SHOW_EVENT_PIC_SIZE);
		delete[] pCh;
		pCh = NULL;

		m_lcAlarmInfo.DeleteItem(SHOW_EVENT_PIC_SIZE);
	}
}
BOOL CAlarmSubscribe::PreTranslateMessage(MSG* pMsg)
{
	// Escape key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CAlarmSubscribe::OnLvnItemchangedListRealAttchinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	NM_LISTVIEW* pNMListView  = (NM_LISTVIEW*)pNMHDR;

	int nItem = pNMListView->iItem;

	if (nItem < 0 ||  nItem >= m_lcAlarmInfo.GetItemCount())
	{
		return ;
	}

	char* pGuid = (char*)m_lcAlarmInfo.GetItemData(nItem);

	std::list<EventPicInfo*>::iterator	 it = m_EventPicList.begin();
	for (; it != m_EventPicList.end(); ++it)
	{
		if (strncmp((*it)->strGuid.c_str(), pGuid, (*it)->strGuid.size()) == 0)
		{
			CRect rect;
			m_dlgShowPic.GetWindowRect(&rect);

			m_dlgShowPic.SetWindowPos(NULL, rect.top,rect.top,0,0,SWP_NOSIZE);
			m_dlgShowPic.ShowWindow(TRUE);
			m_dlgShowPic.ShowPicture((*it)->pPicBuf,(*it)->dwBufSize);

			break;
		}
	}

	*pResult = 0;
}

void CAlarmSubscribe::DeleteAllItemInfo()
{
	int nItemCount = m_lcAlarmInfo.GetItemCount();
	for ( int i = nItemCount - 1; i >= 0; i-- )
	{
		char* pCh = (char*) m_lcAlarmInfo.GetItemData(i);
		delete[] pCh;
		pCh = NULL;
	}
	m_lcAlarmInfo.DeleteAllItems();
}

void CAlarmSubscribe::DeleteAllStoreEventData()
{
	std::list<EventPicInfo*>::iterator it = m_EventPicList.begin();

	while (it != m_EventPicList.end())
	{
		EventPicInfo *pInfo = (*it);
		if (pInfo)
		{
			delete[] pInfo->pPicBuf;
			pInfo->pPicBuf = NULL;

			delete pInfo;
			pInfo = NULL;
		}
		m_EventPicList.erase(it++);
	}

	m_EventPicList.clear();
}
