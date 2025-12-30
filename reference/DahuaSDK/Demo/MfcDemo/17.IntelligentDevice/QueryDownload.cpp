// QueryDownload.cpp : 实现文件
//

#include "stdafx.h"
#include "IntelligentDevice.h"
#include "QueryDownload.h"

#define QUERY_FILE_NUM	20
#define WM_CALLBACK_FUN (WM_USER + 301)

struct	strCallBackInfo
{
	LLONG lHandle;
	DWORD dwTotalSize;
	DWORD dwDownLoadSize;
};

void CALLBACK CallBackFun(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser)
{
	CQueryDownload *dlg = (CQueryDownload*)dwUser;
	if (dlg == NULL)
	{
		return;
	}
	HWND hwnd = dlg->GetSafeHwnd();

	strCallBackInfo *pInfo = NEW strCallBackInfo;
	if (pInfo)
	{
		pInfo->lHandle = lPlayHandle;
		pInfo->dwTotalSize = dwTotalSize;
		pInfo->dwDownLoadSize = dwDownLoadSize;
		::PostMessage(hwnd, WM_CALLBACK_FUN, WPARAM(pInfo), 0);
	}
}

IMPLEMENT_DYNAMIC(CQueryDownload, CDialog)

CQueryDownload::CQueryDownload(int nChannelNum, LLONG lLoginid, CWnd* pParent /*=NULL*/)
	: CDialog(CQueryDownload::IDD, pParent)
	, m_nChannelNum(nChannelNum)
	, m_lLoginid(lLoginid)
	, m_dwCurDownLoadSize(0)
	, m_dwDownLoadTotalSize(0)
	, m_dwCurPlayBackSize(0)
	, m_dwPlayBackTotalSize(0)
	, m_lFindHandle(0)
	, m_nPage(0)
	, m_lDownLoadHandle(0)
	, m_nInfoCount(0)
	, m_QueryFileType(0)
	, m_dwQueryEventType(0)
	, m_playBackHandle(0)
{
	memset(&m_DateStart, 0, sizeof(m_DateStart));
	memset(&m_DateEnd, 0, sizeof(m_DateEnd));
	m_pszSoftPath = NULL;
	m_pszSoftPath= NEW char[1024];
	if (m_pszSoftPath)
	{
		memset(m_pszSoftPath, 0, 1024);
	}
}

CQueryDownload::~CQueryDownload()
{
	if (m_pszSoftPath)
	{
		delete[] m_pszSoftPath;
		m_pszSoftPath = NULL;
	}

	CloseFindHandle();
	CloseDownLoadHandle();
	ClosePlayBackHandle();

	ClearFileInfoVector();
}

void CQueryDownload::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_QUERY_RESULT, m_lcQueryResult);
	DDX_Control(pDX, IDC_PROGRESS_QUERY2, m_PlayBackProcess);
	DDX_Control(pDX, IDC_PROGRESS_QUERY, m_DownLoadProcess);
	DDX_Control(pDX, IDC_COMBO_FILE_TYPE, m_comboFileType);
	DDX_Control(pDX, IDC_COMBO_CHANNEL_QUERY, m_comboQueryChannel);
	DDX_Control(pDX, IDC_COMBO_ALARM_TYPE, m_comboEventType);
}


BEGIN_MESSAGE_MAP(CQueryDownload, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_START, &CQueryDownload::OnBnClickedButtonQueryStart)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_DOWNLOAD, &CQueryDownload::OnBnClickedButtonQueryDownload)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_PREPAGE, &CQueryDownload::OnBnClickedButtonQueryPrepage)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_NEXTPAGE, &CQueryDownload::OnBnClickedButtonQueryNextpage)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_QUERY_RESULT, &CQueryDownload::OnLvnItemchangedListQueryResult)
	ON_MESSAGE(WM_CALLBACK_FUN, &CQueryDownload::OnDoDownLoad)
END_MESSAGE_MAP()

BOOL CQueryDownload::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitClQueryResult();

	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);

	m_PlayBackProcess.SetRange32(0,(int)(m_dwPlayBackTotalSize));
	m_PlayBackProcess.SetPos(0);

	m_DownLoadProcess.SetRange32(0, (int)m_dwDownLoadTotalSize/1024);
	m_DownLoadProcess.SetPos(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CQueryDownload::InitClQueryResult()
{
	m_lcQueryResult.SetExtendedStyle(m_lcQueryResult.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lcQueryResult.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,100,-1);
	m_lcQueryResult.InsertColumn(1,ConvertString("Channel"),LVCFMT_LEFT,100,0);
	m_lcQueryResult.InsertColumn(2,ConvertString("StartTime"),LVCFMT_LEFT,200,1);
	m_lcQueryResult.InsertColumn(3,ConvertString("EndTime"),LVCFMT_LEFT,200,2);

	m_comboFileType.ResetContent();
	m_comboFileType.AddString(ConvertString("Video"));
	m_comboFileType.SetItemData(0, DWORD_PTR(2));
	//m_comboFileType.AddString(ConvertString("Picture"));
	//m_comboFileType.SetItemData(1, DWORD_PTR(1));
	m_comboFileType.SetCurSel(0);

	m_comboEventType.ResetContent();
	CString tmp  = ConvertString(_T("Tripwire"));
	m_comboEventType.AddString(tmp);
	m_comboEventType.SetItemData(0, (DWORD_PTR)EVENT_IVS_CROSSLINEDETECTION);
	tmp = ConvertString(_T("Intrusion"));
	m_comboEventType.AddString(tmp);
	m_comboEventType.SetItemData(1, (DWORD_PTR)EVENT_IVS_CROSSREGIONDETECTION);

	m_comboEventType.SetCurSel(0);

	m_comboQueryChannel.ResetContent();
	m_comboQueryChannel.AddString(ConvertString("All"));
	for(int i=0;i<m_nChannelNum;i++)
	{
		CString str;
		str.Format("%d",i + 1);
		m_comboQueryChannel.AddString(str);
	}
	m_comboQueryChannel.SetCurSel(0);
}

void CQueryDownload::OnBnClickedButtonQueryStart()
{
	CloseFindHandle();
	ClearFileInfoVector();

	m_PlayBackProcess.SetRange32(0,(int)(m_dwPlayBackTotalSize));
	m_PlayBackProcess.SetPos(0);

	m_DownLoadProcess.SetRange32(0,(int)(m_dwDownLoadTotalSize/1024));
	m_DownLoadProcess.SetPos(0);

	m_lcQueryResult.DeleteAllItems();
	m_nInfoCount = 0;
	m_nPage = 0;
	int nChannel = m_comboQueryChannel.GetCurSel() - 1;

	int nIndex = m_comboFileType.GetCurSel();
	if (nIndex == CB_ERR)
	{
		return;
	}
	m_QueryFileType = m_comboFileType.GetItemData(nIndex);

	nIndex = m_comboEventType.GetCurSel();
	if (nIndex == CB_ERR)
	{
		return;
	}
	m_dwQueryEventType = m_comboEventType.GetItemData(nIndex);

	//set up query conditions
	BOOL bRet = DoFindFileEx(nChannel, m_QueryFileType, m_dwQueryEventType);
	if (bRet == FALSE)
	{
		GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);
		return;
	}


	GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
	//do query
	int nQueryCount = DoFindNextFileEx();
	if (nQueryCount > 0)
	{
		UpdateLcQueryList(nQueryCount);
		GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
		if (nQueryCount < QUERY_FILE_NUM)
		{
			GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
		}
		else
		{
			//Second query (cash)
			int nSecondQueryCount = DoFindNextFileEx();
			if (nSecondQueryCount == 0)
			{
				GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(TRUE);
			}
		}
	}
}

BOOL CQueryDownload::DoFindFileEx(int nChannel, int nMediaType, DWORD dwEventType )
{
	NET_IN_MEDIA_QUERY_FILE stMediaIVSEventparam;
	memset(&stMediaIVSEventparam, 0, sizeof(stMediaIVSEventparam));

	stMediaIVSEventparam.dwSize = sizeof(stMediaIVSEventparam);
	
	stMediaIVSEventparam.nMediaType = nMediaType;					//1: picture 2:video
	stMediaIVSEventparam.nChannelID = nChannel;					//-1 means all channel
	stMediaIVSEventparam.nEventLists[0] =  dwEventType;
	stMediaIVSEventparam.nEventCount = 1;					//number of events 				
	stMediaIVSEventparam.emFalgLists[0]= FLAG_TYPE_EVENT;	
	stMediaIVSEventparam.nFalgCount = 1;	

	//set the begin time
	SetData(stMediaIVSEventparam.stuStartTime, IDC_DATE_START);
	SetTime(stMediaIVSEventparam.stuStartTime, IDC_TIME_START);

	//set the end time
	SetData(stMediaIVSEventparam.stuEndTime, IDC_DATE_END);
	SetTime(stMediaIVSEventparam.stuEndTime, IDC_TIME_END);

	m_lFindHandle = CLIENT_FindFileEx(m_lLoginid, DH_FILE_QUERY_FILE, &stMediaIVSEventparam, NULL,DEFAULT_WAIT_TIME);
	if (m_lFindHandle == 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

/************************************************************************/
/*return : query count                                       */
/************************************************************************/
int CQueryDownload::DoFindNextFileEx()
{
	BOOL bRet = FALSE;
	int nRetCount = -1;

	int nSearchCount = QUERY_FILE_NUM;

	NET_OUT_MEDIA_QUERY_FILE *pMediaFileInfo = NEW NET_OUT_MEDIA_QUERY_FILE[nSearchCount];

	if (pMediaFileInfo == NULL)
	{
		MessageBox(ConvertString("New Memory fail!"), ConvertString("Prompt"));
		return -1;
	}

	memset(pMediaFileInfo, 0, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE));

	for (int i = 0; i < nSearchCount; i++)
	{
		pMediaFileInfo[i].dwSize = sizeof(NET_OUT_MEDIA_QUERY_FILE);
	}

	int nCount = CLIENT_FindNextFileEx(m_lFindHandle, nSearchCount, (void*)pMediaFileInfo, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE), NULL,DEFAULT_WAIT_TIME);
	nRetCount = nCount;
	if (nCount < 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
	}

	else if (0 == nCount)
	{
		//MessageBox(ConvertString("Query Number zero"), ConvertString("Prompt"));
	}
	else
	{
		m_nInfoCount += nCount;
		DealDoFindResultInfo(pMediaFileInfo, nCount);
	}

	if (NULL != pMediaFileInfo)
	{
		delete []pMediaFileInfo;
		pMediaFileInfo = NULL;
	}
	return nRetCount;
}

void CQueryDownload::DealDoFindResultInfo(NET_OUT_MEDIA_QUERY_FILE *pMediaFileInfo, int nCount)
{
	for (int i = 0; i < nCount; i++)
	{
		NET_OUT_MEDIA_QUERY_FILE* pInfo = &pMediaFileInfo[i];

		STR_RECORDFILE_INFO *recordfile = NEW STR_RECORDFILE_INFO;
		if (recordfile == NULL)
		{
			MessageBox(ConvertString("New Memory fail!"), ConvertString("Prompt"));
			return;
		}
		memset(recordfile, 0, sizeof(STR_RECORDFILE_INFO));
		strncpy(recordfile->filename, pInfo->szFilePath, sizeof(recordfile->filename) - 1);
		recordfile->size = pInfo->nFileSizeEx;

		recordfile->starttime = pInfo->stuStartTime;
		recordfile->endtime = pInfo->stuEndTime;
		recordfile->driveno = pInfo->nDriveNo;
		recordfile->startcluster = pInfo->nCluster;
		recordfile->ch = pInfo->nChannelID;

		//video
		if (m_QueryFileType == 2)
		{
			recordfile->nRecordFileType = 0;
		}

		//picture
		else if (m_QueryFileType == 1)
		{
			recordfile->nRecordFileType = 4;
		}

		m_DownLoadInfoVector.push_back(recordfile);
	}
}

void CQueryDownload::CloseFindHandle()
{
	if (m_lFindHandle)
	{
		CLIENT_FindCloseEx(m_lFindHandle);
	}
	m_lFindHandle = 0;
}

void CQueryDownload::CloseDownLoadHandle()
{
	if (m_lDownLoadHandle)
	{
		CLIENT_StopDownload(m_lDownLoadHandle);
	}
	m_lDownLoadHandle = 0;
}

void CQueryDownload::ClosePlayBackHandle()
{
	if (m_playBackHandle)
	{
		CLIENT_StopPlayBack(m_playBackHandle);
	}
	m_playBackHandle = 0;
}

void CQueryDownload::OnDoDownload(int nSel)
{
	NET_RECORDFILE_INFO recordfile;
	memset(&recordfile, 0, sizeof(recordfile));

	STR_RECORDFILE_INFO* pInfo = m_DownLoadInfoVector[m_nPage * QUERY_FILE_NUM + nSel];
	strncpy(recordfile.filename, pInfo->filename, sizeof(recordfile.filename) - 1);

	recordfile.size = pInfo->size;

	recordfile.starttime = pInfo->starttime;
	recordfile.endtime = pInfo->endtime;
	recordfile.driveno = pInfo->driveno;
	recordfile.startcluster = pInfo->startcluster;
	recordfile.nRecordFileType = pInfo->nRecordFileType;		
	recordfile.bHint = pInfo->bHint;
	recordfile.ch = pInfo->ch;

	CString strFilePathName;
	if (recordfile.nRecordFileType == 0)
	{
		CFileDialog dlg(FALSE,"*.dav","download.dav",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
			"dav(*.dav)|*.dav|All File(*.dav)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{
			strFilePathName = dlg.GetPathName();
		} 
		else
		{
			GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
			return;
		}
	}
	else if (recordfile.nRecordFileType == 4)
	{
		CFileDialog dlg(FALSE,"*.jpg","download.jpg",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
			"jpg(*.jpg)|*.jpg|All File(*.jpg)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{
			strFilePathName = dlg.GetPathName();
		} 
		else
		{
			GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
			return;
		}
	}
	
	char *pFileName = NULL;
	pFileName = strFilePathName.GetBuffer(200);

	m_lDownLoadHandle = CLIENT_DownloadByRecordFileEx(m_lLoginid, &recordfile, pFileName, CallBackFun, 
		(LDWORD)this, NULL, NULL);

	if (m_lDownLoadHandle == 0)
	{
		MessageBox(ConvertString("DownLoad file fail"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
	}
	strFilePathName.ReleaseBuffer();
	return ;
}

void CQueryDownload::OnBnClickedButtonQueryDownload()
{
	CloseDownLoadHandle();
	CListCtrl *pQueryResult = (CListCtrl*)GetDlgItem(IDC_LIST_QUERY_RESULT);
	int nSel = pQueryResult->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a download file"), ConvertString("Prompt"));
		return;
	}

	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(FALSE);

	//download file
	OnDoDownload(nSel);
}

void CQueryDownload::ClearFileInfoVector()
{
	std::vector<STR_RECORDFILE_INFO*>::iterator it = m_DownLoadInfoVector.begin();
	while(it != m_DownLoadInfoVector.end())
	{
		STR_RECORDFILE_INFO *pstGroupInfo = *it;
		if (pstGroupInfo)
		{
			it = m_DownLoadInfoVector.erase(it);
			delete pstGroupInfo;
			pstGroupInfo = NULL;
		}
	}
	m_DownLoadInfoVector.clear();
}

void CQueryDownload::UpdateLcQueryList(int nInfoNum)
{
	CString strIndex = "";
	CString strStartTime = "";
	CString strEndTime = "";
	CString strChannel = "";
	m_lcQueryResult.DeleteAllItems();

	for (int n = 0; n < nInfoNum; n++)
	{
		strIndex.Format("%d", m_nPage * QUERY_FILE_NUM + n + 1);
		STR_RECORDFILE_INFO *pInfo = m_DownLoadInfoVector[m_nPage * QUERY_FILE_NUM + n];
		strChannel.Format("%d", pInfo->ch + 1);
		strStartTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->starttime.dwYear,
			pInfo->starttime.dwMonth,
			pInfo->starttime.dwDay,
			pInfo->starttime.dwHour,
			pInfo->starttime.dwMinute,
			pInfo->starttime.dwSecond);

		strEndTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->endtime.dwYear,
			pInfo->endtime.dwMonth,
			pInfo->endtime.dwDay,
			pInfo->endtime.dwHour,
			pInfo->endtime.dwMinute,
			pInfo->endtime.dwSecond);
		m_lcQueryResult.InsertItem(n, strIndex);
		m_lcQueryResult.SetItemText(n, 1, strChannel);
		m_lcQueryResult.SetItemText(n, 2, strStartTime);
		m_lcQueryResult.SetItemText(n, 3, strEndTime);
	}
}

void CQueryDownload::OnBnClickedButtonQueryPrepage()
{
	m_nPage--;
	UpdateLcQueryList(QUERY_FILE_NUM);
	if (m_nPage <= 0)
	{
		GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);
	}
	GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(TRUE);
}

void CQueryDownload::OnBnClickedButtonQueryNextpage()
{
	m_nPage++;

	int nSize = m_DownLoadInfoVector.size();

	int nSearchCount = 0;
	if ( (m_nPage+1) * QUERY_FILE_NUM == nSize )
	{
		int nQueryCount = DoFindNextFileEx();
		if (nQueryCount == 0)
		{
			GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
		}
		UpdateLcQueryList(QUERY_FILE_NUM);
	}
	else if((m_nPage+1) * QUERY_FILE_NUM > nSize)
	{
		nSearchCount = nSize - m_nPage * QUERY_FILE_NUM;
		UpdateLcQueryList(nSearchCount);
		GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
	}
	else if ((m_nPage+1) * QUERY_FILE_NUM < nSize)
	{
		UpdateLcQueryList(QUERY_FILE_NUM);
	}

	GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(TRUE);
}

void CQueryDownload::OnLvnItemchangedListQueryResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	ClosePlayBackHandle();

	m_dwCurPlayBackSize = 0;
	m_dwPlayBackTotalSize = 0;

	NET_IN_PLAY_BACK_BY_TIME_INFO stNetin;
	NET_OUT_PLAY_BACK_BY_TIME_INFO stNetOut;
	memset(&stNetin, 0, sizeof(NET_IN_PLAY_BACK_BY_TIME_INFO));
	memset(&stNetOut, 0, sizeof(NET_OUT_PLAY_BACK_BY_TIME_INFO));

	int nChannelId = -1;

	int nSel = m_lcQueryResult.GetNextItem(-1, LVNI_SELECTED);

	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a valid file!"), ConvertString("Prompt"));
		*pResult = 0;
		return;
	}

	STR_RECORDFILE_INFO* pInfo = m_DownLoadInfoVector[m_nPage * QUERY_FILE_NUM + nSel];
	stNetin.stStartTime = pInfo->starttime;
	stNetin.stStopTime = pInfo->endtime;

	nChannelId = pInfo->ch;
	CWnd *pWnd = GetDlgItem(IDC_STATIC_REALPLAY_PIC);
	HWND hwnd = pWnd->GetSafeHwnd();
	stNetin.hWnd = hwnd;
	stNetin.nPlayDirection = 0;
	stNetin.cbDownLoadPos = CallBackFun;
	stNetin.dwPosUser = (LDWORD)this;

	if (pInfo->nRecordFileType == 4)
	{
		MessageBox(ConvertString("Only Video Support PlayBack!"), ConvertString("Prompt"));
	}
	else
	{
		m_playBackHandle = CLIENT_PlayBackByTimeEx2(m_lLoginid, nChannelId, &stNetin, &stNetOut);
		if (m_playBackHandle == 0)
		{
			MessageBox(ConvertString("PlayBack fail"), ConvertString("Prompt"));
		}
	}

	*pResult = 0;
}


LRESULT CQueryDownload::OnDoDownLoad(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
	{
		return 0;
	}

	strCallBackInfo *pInfo = (strCallBackInfo *)wParam;
	if (pInfo)
	{
		ReceivePlayPos(pInfo->lHandle, pInfo->dwTotalSize, pInfo->dwDownLoadSize);
		delete pInfo;
		pInfo = NULL;
	}
	return 0;
}

void CQueryDownload::ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize)
{
	if (lPlayHandle == m_playBackHandle)
	{
		if (dwDownLoadSize == -1)
		{
			m_dwCurPlayBackSize = 100;
			m_dwPlayBackTotalSize = 100;
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_REALPLAY_PIC));
			ClosePlayBackHandle();
			//return;
		}
		else
		{
			m_dwPlayBackTotalSize = dwTotalSize;
			m_dwCurPlayBackSize = dwDownLoadSize;
		}
		m_PlayBackProcess.SetRange32(0,(int)(m_dwPlayBackTotalSize));
		m_PlayBackProcess.SetPos((int)(m_dwCurPlayBackSize) + 1);
	}
	else if (lPlayHandle == m_lDownLoadHandle)
	{
		if (dwDownLoadSize == -1)
		{
			m_dwDownLoadTotalSize = 100 * 1024;
			m_dwCurPlayBackSize = 100;
			CloseDownLoadHandle();
			MessageBox(ConvertString("Download file success!"), ConvertString("Prompt"));
			GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
			//return;
		}
		else
		{
			m_dwDownLoadTotalSize = dwTotalSize;
			m_dwCurDownLoadSize = dwDownLoadSize;
		}
		m_DownLoadProcess.SetRange32(0,(int)(m_dwDownLoadTotalSize/1024));
		m_DownLoadProcess.SetPos((int)(m_dwCurDownLoadSize));
	}
}

void CQueryDownload::SetData(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwYear = stuTime.wYear;
	stNetTime.dwMonth = stuTime.wMonth;
	stNetTime.dwDay = stuTime.wDay;
}

void CQueryDownload::SetTime(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwHour = stuTime.wHour;
	stNetTime.dwMinute = stuTime.wMinute;
	stNetTime.dwSecond = stuTime.wSecond;
}

BOOL CQueryDownload::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	/*if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}*/

	// Escape key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
