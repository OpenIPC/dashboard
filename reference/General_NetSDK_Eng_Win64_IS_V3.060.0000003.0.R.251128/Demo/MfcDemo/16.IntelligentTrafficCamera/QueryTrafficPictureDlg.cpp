// QueryPictureAndRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelligentTrafficCamera.h"
#include "QueryTrafficPictureDlg.h"
#include <cmath>

// CQueryPictureAndRecordDlg Dialog

IMPLEMENT_DYNAMIC(CQueryTrafficPictureDlg, CDialog)

CQueryTrafficPictureDlg::CQueryTrafficPictureDlg(CWnd* pParent ,int nChannel,LLONG lLoginID)
: CDialog(CQueryTrafficPictureDlg::IDD, pParent)
{
	m_nCurPage = 0;
	m_nTotalPage = 0;
	m_nIndexOfTrafficPicture = 0;
	m_nTotalCountOfTrafficPicture = 0;

	m_lLoginHandle = lLoginID;
	m_nChannel = nChannel;
	m_lFindPictureHandle = 0;
	m_lDownloadHandle = 0;
}

CQueryTrafficPictureDlg::~CQueryTrafficPictureDlg()
{
	CleanUp();
}

void CQueryTrafficPictureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_CHANNEL, m_cmbChannel);
	DDX_Control(pDX, IDC_BTN_QUERY, m_btnQuery);
	DDX_Control(pDX, IDC_BTN_PTRVIOUS_PAGE, m_btnPreviousPage);
	DDX_Control(pDX, IDC_BTN_NEXT_PAGE, m_btnNextPage);
	DDX_Control(pDX, IDC_LIST_PICTURE, m_ctrPicture);
	DDX_Control(pDX, IDC_CMB_EVENT_TYPE, m_cmbEventType);
	DDX_Control(pDX, IDC_BTN_DOWNLOAD, m_btnDownload);
}


BEGIN_MESSAGE_MAP(CQueryTrafficPictureDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_QUERY, &CQueryTrafficPictureDlg::OnBnClickedBtnQuery)
	ON_BN_CLICKED(IDC_BTN_PTRVIOUS_PAGE, &CQueryTrafficPictureDlg::OnBnClickedBtnPtrviousPage)
	ON_BN_CLICKED(IDC_BTN_NEXT_PAGE, &CQueryTrafficPictureDlg::OnBnClickedBtnNextPage)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD, &CQueryTrafficPictureDlg::OnBnClickedBtnDownload)
	ON_MESSAGE(WM_PICTURE_DOWNLOAD_POS, &CQueryTrafficPictureDlg::OnDownloadPicturePos)
	ON_WM_DESTROY()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_START_DATE, &CQueryTrafficPictureDlg::OnDtnDatetimechangeDatetimepickerStartDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_END_DATE, &CQueryTrafficPictureDlg::OnDtnDatetimechangeDatetimepickerEndDate)
END_MESSAGE_MAP()


// CQueryPictureAndRecordDlg message handlers

BOOL CQueryTrafficPictureDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	// Escape key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CQueryTrafficPictureDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	for (unsigned int i = 0 ;i < m_nChannel; i++)
	{
		CString csChannel;
		csChannel.Format("%d", i+1);
		m_cmbChannel.InsertString(i, csChannel);
	}
	m_cmbChannel.SetCurSel(0);
	for (int i = 0; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		m_cmbEventType.InsertString(i, ConvertString(strEventList[i]));
	}
	// Set event

	m_cmbEventType.SetCurSel(0);

	m_btnQuery.EnableWindow(TRUE);
	m_btnPreviousPage.EnableWindow(FALSE);
	m_btnNextPage.EnableWindow(FALSE);
	m_btnDownload.EnableWindow(FALSE);
	m_ctrPicture.DeleteAllItems();

	m_ctrPicture.SetExtendedStyle(m_ctrPicture.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctrPicture.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 60);
	m_ctrPicture.InsertColumn(1, ConvertString("Lane"), LVCFMT_LEFT, 70);
	m_ctrPicture.InsertColumn(2, ConvertString("Size(KB)"), LVCFMT_LEFT, 80);
	m_ctrPicture.InsertColumn(3, ConvertString("Time"), LVCFMT_LEFT, 120);
	m_ctrPicture.InsertColumn(4, ConvertString("Plate Number"), LVCFMT_LEFT, 120);
	m_ctrPicture.InsertColumn(5, ConvertString("Plate Color"), LVCFMT_LEFT, 100);
	m_ctrPicture.InsertColumn(6, ConvertString("Vehicle Color"), LVCFMT_LEFT, 100);
	m_ctrPicture.InsertColumn(8, ConvertString("Speed(km/h)"), LVCFMT_LEFT, 100);
	m_ctrPicture.InsertColumn(9, ConvertString("Event Type"), LVCFMT_LEFT, 120);
	m_ctrPicture.InsertColumn(10, ConvertString("Vehicle Size"), LVCFMT_LEFT, 100);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CQueryTrafficPictureDlg::CleanUp()
{
	for (unsigned int i =0; i < m_vecTrafficPicture.size(); i++)
	{
		MEDIAFILE_TRAFFICCAR_INFO_EX* pTrafficPicture = m_vecTrafficPicture[i];
		if (NULL != pTrafficPicture)
		{
			delete pTrafficPicture;
			pTrafficPicture = NULL;
		}
	}
	m_vecTrafficPicture.clear();

	if (0 != m_lFindPictureHandle)
	{
		CLIENT_FindCloseEx(m_lFindPictureHandle);
	}

	if (0 != m_lDownloadHandle)
	{
		CLIENT_StopLoadMultiFile(m_lDownloadHandle);
	}

	//m_cmbChannel.ResetContent();
	//m_cmbEventType.ResetContent();
	//m_btnQuery.EnableWindow(FALSE);
	//m_btnPreviousPage.EnableWindow(FALSE);
	//m_btnNextPage.EnableWindow(FALSE);
	//m_btnDownload.EnableWindow(FALSE);
	//m_ctrPicture.DeleteAllItems();
}


void CQueryTrafficPictureDlg::OnBnClickedBtnQuery()
{
	// Whether the begin time is less than end time
	bool bRet = IsTimeCorrent();
	if (!bRet)
	{
		MessageBox(ConvertString("The begin time is bigger than end time, please input again!"), ConvertString("Prompt"));
		return;
	}

	CleanUpResultOfLastTimeQuery();

	m_lFindPictureHandle = StartFindPictures();
	if (0 == m_lFindPictureHandle)
	{
		return;
	}

	m_nTotalCountOfTrafficPicture = GetTotalCountOfPicture();
	if (m_nTotalCountOfTrafficPicture <= 0)
	{
		return;
	}

	m_nCurPage = 1;
	SetDlgItemInt(IDC_EDIT_RECORD_COUNT, m_nTotalCountOfTrafficPicture);
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);

	int nQueryCount = SINGLE_QUERY_COUNT;
	if (m_nTotalCountOfTrafficPicture <= SINGLE_QUERY_COUNT)
	{
		nQueryCount = m_nTotalCountOfTrafficPicture ;
		m_nTotalPage = 1;
	}
	else
	{
		m_btnNextPage.EnableWindow(TRUE);
		m_nTotalPage = (int)ceil(m_nTotalCountOfTrafficPicture / (float)nQueryCount);
	}

	if (!DoFindPicutres(nQueryCount))
	{
		m_btnNextPage.EnableWindow(FALSE);
		return;
	}
	m_btnDownload.EnableWindow(TRUE);
}

void CQueryTrafficPictureDlg::CleanUpResultOfLastTimeQuery()
{
	m_nTotalPage = 0;
	m_nCurPage = 0;
	m_nIndexOfTrafficPicture = 0;
	m_nTotalCountOfTrafficPicture = 0;

	m_ctrPicture.DeleteAllItems();
	m_btnNextPage.EnableWindow(FALSE);
	m_btnPreviousPage.EnableWindow(FALSE);
	SetDlgItemText(IDC_EDIT_RECORD_COUNT, "");
	SetDlgItemText(IDC_EDIT_RECORD_CURRENT_PAGE, "");


	for (unsigned int i =0; i < m_vecTrafficPicture.size(); i++)
	{
		MEDIAFILE_TRAFFICCAR_INFO_EX* pTrafficPicture = m_vecTrafficPicture[i];
		if (NULL != pTrafficPicture)
		{
			delete pTrafficPicture;
			pTrafficPicture = NULL;
		}
	}
	m_vecTrafficPicture.clear();

	if (0 != m_lFindPictureHandle)
	{
		CLIENT_FindCloseEx(m_lFindPictureHandle);
		m_lFindPictureHandle = 0;
	}
}

LLONG CQueryTrafficPictureDlg::StartFindPictures(  )
{
	CString strPlateNumber;
	GetDlgItemText(IDC_EDIT_PLATE_NUMBER, strPlateNumber);  
	int nSelect = m_cmbEventType.GetCurSel();
	if (nSelect == -1)
	{
		return 0;
	}

	MEDIA_QUERY_TRAFFICCAR_PARAM_EX stQueryTrafficParam = {sizeof(MEDIA_QUERY_TRAFFICCAR_PARAM_EX )};

	int nEventType = 0;

	if (nSelect == 0)
	{
		stQueryTrafficParam.stuParam.pEventTypes = NULL; 
	}
	else
	{
		nEventType = (int)nEventTypeList[nSelect];
	}

	if (0 != nSelect)
	{
		stQueryTrafficParam.stuParam.pEventTypes = &nEventType;
		stQueryTrafficParam.stuParam.nEventTypeNum = 1;
	}

	stQueryTrafficParam.stuParam.nChannelID = m_cmbChannel.GetCurSel();
	stQueryTrafficParam.stuParam.nMediaType = 1;
	stQueryTrafficParam.stuParam.byLane = -1;
	strncpy(stQueryTrafficParam.stuParam.szPlateNumber, strPlateNumber.GetBuffer(), sizeof(stQueryTrafficParam.stuParam.szPlateNumber)-1);  // Plate Number
	CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE);
	CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME);
	CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE);
	CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);
	GetTimeFromTimeCtr(stQueryTrafficParam.stuParam.StartTime, pCtrBeginDate, pCtrBeginTime);
	GetTimeFromTimeCtr(stQueryTrafficParam.stuParam.EndTime, pCtrEndtDate, pCtrEndTime);
	strncpy(stQueryTrafficParam.stuParam.szPlateNumber, strPlateNumber, sizeof(stQueryTrafficParam.stuParam.szPlateNumber)-1);

	LLONG lFindHanle = CLIENT_FindFileEx(m_lLoginHandle, DH_FILE_QUERY_TRAFFICCAR_EX, &stQueryTrafficParam, NULL, MAX_TIMEOUT);
	if (0 == lFindHanle)
	{
		MessageBox(ConvertString("Query picture failed!"), ConvertString("Prompt"));
		return 0;
	}
	return lFindHanle;
}

int CQueryTrafficPictureDlg::GetTotalCountOfPicture()
{
	int nCount;
	BOOL bRet = CLIENT_GetTotalFileCount(m_lFindPictureHandle, &nCount, NULL, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Get total count of picture failed!"), ConvertString("Prompt"));
		return -1;
	}
	if (nCount == 0)
	{
		MessageBox(ConvertString("Find Zero!"), ConvertString("Prompt"));
	}
	return nCount;
}

bool CQueryTrafficPictureDlg::DoFindPicutres( int nQueryCount )
{
	MEDIAFILE_TRAFFICCAR_INFO_EX* pTrafficParam = new MEDIAFILE_TRAFFICCAR_INFO_EX[nQueryCount];
	memset(pTrafficParam, 0, sizeof(MEDIAFILE_TRAFFICCAR_INFO_EX)* nQueryCount);

	for (int i= 0; i<nQueryCount; i++)
	{
		pTrafficParam[i].dwSize = sizeof(MEDIAFILE_TRAFFICCAR_INFO_EX);
	}

	int nRet = CLIENT_FindNextFileEx(m_lFindPictureHandle, nQueryCount, pTrafficParam, sizeof(MEDIAFILE_TRAFFICCAR_INFO_EX)* nQueryCount, NULL, MAX_TIMEOUT);
	if (0 == nRet)
	{
		MessageBox(ConvertString("Query picture failed!"), ConvertString("Prompt"));
		delete[] pTrafficParam;
		return false;
	}

	for (int i = 0; i < nRet; i++)
	{ 
		m_nIndexOfTrafficPicture++;
		ShowTrafficPictureInfo(pTrafficParam+i, m_nIndexOfTrafficPicture);

		MEDIAFILE_TRAFFICCAR_INFO_EX* pTrafficListInfo = new MEDIAFILE_TRAFFICCAR_INFO_EX;
		memcpy(pTrafficListInfo, pTrafficParam+i, sizeof(MEDIAFILE_TRAFFICCAR_INFO_EX));        
		m_vecTrafficPicture.push_back(pTrafficListInfo);
	}
	delete[] pTrafficParam;
	return true;
}


void CQueryTrafficPictureDlg::ShowTrafficPictureInfo(MEDIAFILE_TRAFFICCAR_INFO_EX* pRecordList, int nBeginIndex)
{
	if (NULL == pRecordList || nBeginIndex <= 0)
	{
		return;
	}

	CString strIndex;
	CString strLane;
	CString strSize;
	CString strTime;
	CString strSpeed;
	CString strEvent;

	strIndex.Format("%d", nBeginIndex);
	strLane.Format("%d", pRecordList->stuInfo.byLane);
	strSize.Format("%d", pRecordList->stuInfo.sizeEx / 1024);
	strSpeed.Format("%d", pRecordList->stuInfo.nSpeed);

	strTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
		pRecordList->stuInfo.starttime.dwYear, pRecordList->stuInfo.starttime.dwMonth, 
		pRecordList->stuInfo.starttime.dwDay, pRecordList->stuInfo.starttime.dwHour, 
		pRecordList->stuInfo.starttime.dwMinute, pRecordList->stuInfo.starttime.dwSecond);

	int n =pRecordList->stuInfo.nEvents[0];

	strEvent = ConvertString("Unknown");
	for (int i = 1; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		int nEven = (int)nEventTypeList[i];
		if (n == nEven)
		{
			strEvent = ConvertString(strEventList[i]);
			break;
		}
	}

	int nCount =  m_ctrPicture.GetItemCount();
	m_ctrPicture.InsertItem(LVIF_TEXT|LVIF_STATE,nCount,strIndex,0,LVIS_SELECTED,0,0);
	m_ctrPicture.SetItemText(nCount,0, strIndex);
	m_ctrPicture.SetItemText(nCount,1, strLane);
	m_ctrPicture.SetItemText(nCount,2, strSize);
	m_ctrPicture.SetItemText(nCount,3, strTime);
	m_ctrPicture.SetItemText(nCount,4, pRecordList->stuInfo.szPlateNumber);
	m_ctrPicture.SetItemText(nCount,5, pRecordList->stuInfo.szPlateColor);
	m_ctrPicture.SetItemText(nCount,6, pRecordList->stuInfo.szVehicleColor);
	m_ctrPicture.SetItemText(nCount,7, strSpeed);
	m_ctrPicture.SetItemText(nCount,8, strEvent);
	m_ctrPicture.SetItemText(nCount,9, pRecordList->stuInfo.szVehicleSize); 
}

void CQueryTrafficPictureDlg::GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime)
{
	if (NULL == pCtrDate || NULL == pCtrTime)
	{
		return;
	}

	COleDateTime tmDate;
	COleDateTime tmTime;
	pCtrDate->GetTime(tmDate);
	pCtrTime->GetTime(tmTime);

	stTime.dwYear = tmDate.GetYear();
	stTime.dwMonth = tmDate.GetMonth();
	stTime.dwDay = tmDate.GetDay();
	stTime.dwHour = tmTime.GetHour();
	stTime.dwMinute = tmTime.GetMinute();
	stTime.dwSecond = tmTime.GetSecond();
}
void CQueryTrafficPictureDlg::OnBnClickedBtnPtrviousPage()
{
	if(1 >= m_nCurPage)
	{
		return;
	}

	m_ctrPicture.DeleteAllItems();
	for (int i = 0; i < SINGLE_QUERY_COUNT; i++)
	{
		ShowTrafficPictureInfo(m_vecTrafficPicture[(m_nCurPage-2)*SINGLE_QUERY_COUNT+i], (m_nCurPage-2)*SINGLE_QUERY_COUNT+i+1);
	}
	m_nCurPage--;
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);
	if (m_nCurPage <= 1)
	{
		m_btnPreviousPage.EnableWindow(FALSE);
	}
	m_btnNextPage.EnableWindow(TRUE);
}

void CQueryTrafficPictureDlg::OnBnClickedBtnNextPage()
{
	if(0 >= m_nCurPage)
	{
		return;
	}
	m_ctrPicture.DeleteAllItems();
	if (m_vecTrafficPicture.size() > m_nCurPage*SINGLE_QUERY_COUNT)
	{
		int nRemainCount = m_vecTrafficPicture.size() - m_nCurPage*SINGLE_QUERY_COUNT;
		int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT?SINGLE_QUERY_COUNT:nRemainCount;
		for (int i = 0; i < nQueryCount; i++)
		{
			ShowTrafficPictureInfo(m_vecTrafficPicture[m_nCurPage*SINGLE_QUERY_COUNT+i], m_nCurPage*SINGLE_QUERY_COUNT+i+1);
		}
	}
	else
	{
		int nRemainCount = m_nTotalCountOfTrafficPicture- m_nCurPage*SINGLE_QUERY_COUNT;
		int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT?SINGLE_QUERY_COUNT:nRemainCount;
		if (!DoFindPicutres(nQueryCount))
		{
			return;
		}
	} 
	m_nCurPage++;
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);
	if (m_nCurPage >= m_nTotalPage)
	{
		m_btnNextPage.EnableWindow(FALSE);
	}

	m_btnPreviousPage.EnableWindow(TRUE);
}


// 下载进度回调函数原型, nError表示在下载过程出现的错误,1-缓存不足,2-对返回数据的校验出错,3-下载当前文件失败,4-创建对应保存文件失败
// 按文件下载时,ID为用户设置的dwFileID,按条件下载时,ID为用户设置的dwConditionID                    
void CALLBACK DownloadTrafficPicture(LLONG lDownLoadHandle, DWORD dwID, DWORD dwFileTotalSize, DWORD dwDownLoadSize, int nError, LDWORD dwUser, void* pReserved)
{
	if (nError != 0 || dwDownLoadSize == UINT_MAX)
	{
		PostMessage(((CQueryTrafficPictureDlg *)dwUser)->GetSafeHwnd(), WM_PICTURE_DOWNLOAD_POS, 0, 0);
	}
}

LRESULT CQueryTrafficPictureDlg::OnDownloadPicturePos(WPARAM wParam, LPARAM lParam)
{
	CLIENT_StopLoadMultiFile(m_lDownloadHandle);

	m_btnDownload.EnableWindow(TRUE);
	return 1;
}

void CQueryTrafficPictureDlg::OnBnClickedBtnDownload()
{

	int nSelect = m_ctrPicture.GetNextItem(-1,LVNI_SELECTED);
	if(nSelect < 0)
	{
		MessageBox(ConvertString(ConvertString("Please select a list!")),  ConvertString("Prompt"));
		return;
	}

	CString strFilePathName;
	CFileDialog dlg(FALSE,"*.jpg","download.jpg",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"jpg(*.jpg)|*.jpg|All File(*.jpg)|*.*||",this);
	if(dlg.DoModal() == IDOK)
	{
		strFilePathName = dlg.GetPathName();
	}
	else
	{
		return;
	}

	m_btnDownload.EnableWindow(FALSE);

	int nIndex = (m_nCurPage-1)*SINGLE_QUERY_COUNT + nSelect;
	NET_DOWNLOADFILE_INFO stDownloadInfo = {sizeof(NET_DOWNLOADFILE_INFO)};
	stDownloadInfo.dwFileID = 1;
	stDownloadInfo.nFileSize = m_vecTrafficPicture[nIndex]->stuInfo.sizeEx / 1024;
	strncpy(stDownloadInfo.szSourceFilePath, m_vecTrafficPicture[nIndex]->stuInfo.szFilePath, MAX_PATH-1);
	strncpy(stDownloadInfo.szSavedFileName, strFilePathName.GetBuffer(), MAX_PATH-1);


	NET_IN_DOWNLOAD_MULTI_FILE stInDownloadFile = {sizeof(NET_IN_DOWNLOAD_MULTI_FILE )};
	stInDownloadFile.emDownloadType = EM_DOWNLOAD_BY_FILENAME;
	stInDownloadFile.cbPosCallBack = DownloadTrafficPicture;
	stInDownloadFile.dwUserData =  (LDWORD)this;
	stInDownloadFile.nFileCount = 1;
	stInDownloadFile.pFileInfos = &stDownloadInfo;
	NET_OUT_DOWNLOAD_MULTI_FILE stOutDownloadFile= {sizeof(NET_OUT_DOWNLOAD_MULTI_FILE )};

	BOOL nRet = CLIENT_DownLoadMultiFile(m_lLoginHandle, &stInDownloadFile, &stOutDownloadFile, MAX_TIMEOUT);
	if (!nRet)
	{
		MessageBox(ConvertString("Download picture failed!"), ConvertString("Prompt"));
	}
	m_lDownloadHandle = stOutDownloadFile.lDownLoadHandle;
}

bool CQueryTrafficPictureDlg::IsTimeCorrent()
{
	CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE);
	CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME);
	CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE);
	CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);

	COleDateTime beginDate;
	COleDateTime beginTime;
	COleDateTime endDate;
	COleDateTime endTime;
	pCtrBeginDate->GetTime(beginDate);
	pCtrBeginTime->GetTime(beginTime);
	pCtrEndtDate->GetTime(endDate);
	pCtrEndTime->GetTime(endTime);

	if (endDate >= beginDate)
	{
		if (endDate == beginDate)
		{
			if (endTime < beginTime)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}



void CQueryTrafficPictureDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CleanUpResultOfLastTimeQuery();
}

void CQueryTrafficPictureDlg::OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	if (2000 > pDTChange->st.wYear || 2038 < pDTChange->st.wYear)
	{
		MessageBox(ConvertString("The time range is from 2000 to 2038.Please input again!"), ConvertString("Prompt"));
		COleDateTime tmDate  = COleDateTime::GetCurrentTime();
		((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE))->SetTime(tmDate);

	}  
	*pResult = 0;
}

void CQueryTrafficPictureDlg::OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	if (2000 > pDTChange->st.wYear || 2038 < pDTChange->st.wYear)
	{
		MessageBox(ConvertString("The time range is from 2000 to 2038.Please input again!"), ConvertString("Prompt"));
		COleDateTime tmDate  = COleDateTime::GetCurrentTime();
		((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE))->SetTime(tmDate);

	}  
	*pResult = 0;
}
