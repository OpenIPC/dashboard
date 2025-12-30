// TrafficFlowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelligentTrafficCamera.h"
#include "TrafficFlowDlg.h"
#include <cmath>


#define WM_TRAFFIC_FLOW         (WM_USER + 20)   

// CTrafficFlowDlg Dialog

IMPLEMENT_DYNAMIC(CTrafficFlowDlg, CDialog)

CTrafficFlowDlg::CTrafficFlowDlg(CWnd* pParent ,int nChannel,LLONG lLoginID )
: CDialog(CTrafficFlowDlg::IDD, pParent)
{
	m_nTotalPage = 0;
	m_nCurPage = 0;
	m_nIndexOfSubscribeTrafficFlow = 0;
	m_nTotalCountOfTrafficFlow = 0;
	m_nIndexOfQueryTrafficFlow = 0;

	m_lLoginHandle = lLoginID;
	m_nChannel = nChannel;
	m_lSubscribeTrafficFlowHandle = 0;
	m_lQueryTrafficFlowHandle = 0;
}

CTrafficFlowDlg::~CTrafficFlowDlg()
{
	CleanUp();
}

void CTrafficFlowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_SUNSCIRBE_AND_UNSUBSCRIBE, m_btnSubscribe);
	DDX_Control(pDX, IDC_BTN_QUERY, m_btnQuery);
	DDX_Control(pDX, IDC_LIST_TRAFFIC_FLOW, m_ctrTrafficFlow);
	DDX_Control(pDX, IDC_CMB_CHANNEL, m_cmbChannel);
	DDX_Control(pDX, IDC_CMB_LANE, m_cmbLane);
	DDX_Control(pDX, IDC_LIST_QUERY_TRAFFIC_FLOW, m_ctrQueryTrafficFlow);
	DDX_Control(pDX, IDC_BTN_PREVIOUS_PAGE, m_btnPreviousPage);
	DDX_Control(pDX, IDC_BTN_NEXT_PAGE, m_btnNextPage);
}


BEGIN_MESSAGE_MAP(CTrafficFlowDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SUNSCIRBE_AND_UNSUBSCRIBE, &CTrafficFlowDlg::OnBnClickedBtnSunscirbeAndUnsubscribe)
	ON_MESSAGE(WM_TRAFFIC_FLOW, &CTrafficFlowDlg::OnTrafficFlowDate)
	ON_CBN_SELCHANGE(IDC_CMB_LANE, &CTrafficFlowDlg::OnCbnSelchangeCmbLane)
	ON_BN_CLICKED(IDC_BTN_QUERY, &CTrafficFlowDlg::OnBnClickedBtnQuery)
	ON_BN_CLICKED(IDC_BTN_PREVIOUS_PAGE, &CTrafficFlowDlg::OnBnClickedBtnPreviousPage)
	ON_BN_CLICKED(IDC_BTN_NEXT_PAGE, &CTrafficFlowDlg::OnBnClickedBtnNextPage)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_START_DATE, &CTrafficFlowDlg::OnDtnDatetimechangeDatetimepickerStartDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_END_DATE, &CTrafficFlowDlg::OnDtnDatetimechangeDatetimepickerEndDate)
END_MESSAGE_MAP()


// CTrafficFlowDlg message handlers

BOOL CTrafficFlowDlg::PreTranslateMessage(MSG* pMsg)
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

BOOL CTrafficFlowDlg::OnInitDialog()
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

	for (int i =0; i < DH_MAX_LANE_NUM; i++)
	{
		CString strLane;
		strLane.Format("%d", i+1);
		m_cmbLane.InsertString(i, strLane);
	}
	m_cmbLane.SetCurSel(0);

	m_btnSubscribe.EnableWindow(TRUE);
	m_btnQuery.EnableWindow(TRUE);

	m_btnPreviousPage.EnableWindow(FALSE);
	m_btnNextPage.EnableWindow(FALSE);

	m_ctrTrafficFlow.SetExtendedStyle(m_ctrTrafficFlow.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctrTrafficFlow.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 60);
	m_ctrTrafficFlow.InsertColumn(1, ConvertString("Start Time"), LVCFMT_LEFT, 120);
	m_ctrTrafficFlow.InsertColumn(2, ConvertString("Period(Second)"), LVCFMT_LEFT, 90);
	m_ctrTrafficFlow.InsertColumn(3, ConvertString("Flow Count"), LVCFMT_LEFT, 90);
	m_ctrTrafficFlow.InsertColumn(4, ConvertString("Human Count"), LVCFMT_LEFT, 90);
	m_ctrTrafficFlow.InsertColumn(5, ConvertString("Avg Speed(km/h)"), LVCFMT_LEFT, 110);
	m_ctrTrafficFlow.InsertColumn(6, ConvertString("Time Percentage"), LVCFMT_LEFT, 120);
	m_ctrTrafficFlow.InsertColumn(7, ConvertString("Space Occupy Rate"), LVCFMT_LEFT, 120);

	m_ctrQueryTrafficFlow.SetExtendedStyle(m_ctrTrafficFlow.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctrQueryTrafficFlow.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 60);
	m_ctrQueryTrafficFlow.InsertColumn(1, ConvertString("Lane"), LVCFMT_LEFT, 70);
	m_ctrQueryTrafficFlow.InsertColumn(2, ConvertString("Flow Count"), LVCFMT_LEFT, 90);
	m_ctrQueryTrafficFlow.InsertColumn(3, ConvertString("Human Count"), LVCFMT_LEFT, 90);
	m_ctrQueryTrafficFlow.InsertColumn(4, ConvertString("Avg Speed(km/h)"), LVCFMT_LEFT, 120);
	m_ctrQueryTrafficFlow.InsertColumn(5, ConvertString("Time Percentage"), LVCFMT_LEFT, 120);
	m_ctrQueryTrafficFlow.InsertColumn(6, ConvertString("Space Occupy Rate"), LVCFMT_LEFT, 130);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CTrafficFlowDlg::CleanUp()
{
	if (m_lQueryTrafficFlowHandle)
	{
		// Stop query traffic flow
		CLIENT_FindRecordClose(m_lQueryTrafficFlowHandle);
	}

	if (m_lSubscribeTrafficFlowHandle)
	{
		// Unsubscribe traffic flow
		CLIENT_StopLoadPic(m_lSubscribeTrafficFlowHandle);
	}

	for (unsigned int i =0; i < m_vecTrafficFlow.size(); i++)
	{
		NET_RECORD_TRAFFIC_FLOW_STATE* pTrafficFlow = m_vecTrafficFlow[i];
		if (NULL != pTrafficFlow)
		{
			delete pTrafficFlow;
			pTrafficFlow = NULL;
		}
	}
	m_vecTrafficFlow.clear();

	//m_cmbChannel.ResetContent();
	//m_btnQuery.EnableWindow(FALSE);
	//m_btnSubscribe.EnableWindow(FALSE);
	//m_ctrQueryTrafficFlow.DeleteAllItems();
	//SetDlgItemText(IDC_EDIT_RECORD_COUNT, "");
	//SetDlgItemText(IDC_EDIT_RECORD_CURRENT_PAGE, "");
}

int CALLBACK cbTrafficFlowCallBack(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
	if (EVENT_IVS_TRAFFIC_FLOWSTATE != dwAlarmType || dwUser == NULL)
	{
		return -1;
	}
	DEV_EVENT_TRAFFIC_FLOW_STATE* pInfo =  new DEV_EVENT_TRAFFIC_FLOW_STATE;
	memcpy(pInfo, (DEV_EVENT_TRAFFIC_FLOW_STATE*)pAlarmInfo, sizeof(DEV_EVENT_TRAFFIC_FLOW_STATE));
	PostMessage(((CTrafficFlowDlg *)dwUser)->GetSafeHwnd(), WM_TRAFFIC_FLOW, (WPARAM)pInfo, (LPARAM)lAnalyzerHandle);

	return 0;
}

// Show traffic flow info in List control
LRESULT CTrafficFlowDlg::OnTrafficFlowDate(WPARAM wParam, LPARAM lParam)
{
	if (0 == wParam || lParam == 0)
	{
		return 0;
	}
	if (m_lSubscribeTrafficFlowHandle != (LLONG)lParam)
	{
		return 0;
	}
	if (m_ctrTrafficFlow.GetItemCount() > MAX_EVENT_IN_LIST_CONTROL)
	{
		m_ctrTrafficFlow.DeleteItem(MAX_EVENT_IN_LIST_CONTROL);
	}

	DEV_EVENT_TRAFFIC_FLOW_STATE* pInfo = (DEV_EVENT_TRAFFIC_FLOW_STATE*)wParam;
	int nLane = m_cmbLane.GetCurSel();

	for (int i =0; i < DH_MAX_LANE_NUM; i++)
	{
		if ((nLane + 1 )== pInfo->stuStates[i].nLane)
		{
			CString strIndex;
			CString strTime;
			CString strPeriod;
			CString strFlow;
			CString strHumanFlow;
			CString strAvgSpeed;
			CString strTimePercentage;
			CString strSpaceOccupyRate;

			strIndex.Format("%d",++m_nIndexOfSubscribeTrafficFlow);	
			strTime.Format("%04d-%02d-%02d %02d:%02d:%02d", pInfo->UTC.dwYear, pInfo->UTC.dwMonth,
				pInfo->UTC.dwDay, pInfo->UTC.dwHour, pInfo->UTC.dwMinute, pInfo->UTC.dwSecond);
			strPeriod.Format("%d", pInfo->stuStates[i].dwPeriod*60+ (pInfo->stuStates[i].dwPeriodByMili)/1000);// dwPeriodByMili
			strFlow.Format("%d", pInfo->stuStates[i].dwFlow); 
			strHumanFlow.Format("%d", pInfo->stuStates[i].nPasserbyVehicles); 
			if (pInfo->stuStates[i].fAverageSpeed < 0)
			{
				strAvgSpeed.Format("-"); 
			}
			else
			{
				strAvgSpeed.Format("%.1f", pInfo->stuStates[i].fAverageSpeed); 
			}
			strTimePercentage.Format("%.1f", pInfo->stuStates[i].fTimeOccupyRatio);
			strSpaceOccupyRate.Format("%.1f", pInfo->stuStates[i].fSpaceOccupyRatio);


			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = 0;
			m_ctrTrafficFlow.InsertItem(&lvi);
			m_ctrTrafficFlow.SetItemText(0,0, strIndex);
			m_ctrTrafficFlow.SetItemText(0,1, strTime);
			m_ctrTrafficFlow.SetItemText(0,2, strPeriod);
			m_ctrTrafficFlow.SetItemText(0,3, strFlow);
			m_ctrTrafficFlow.SetItemText(0,4, strHumanFlow);
			m_ctrTrafficFlow.SetItemText(0,5, strAvgSpeed);
			m_ctrTrafficFlow.SetItemText(0,6, strTimePercentage);
			m_ctrTrafficFlow.SetItemText(0,7, strSpaceOccupyRate);
			break;
		}
	}

	delete pInfo;
	pInfo = NULL;
	return 0;
}

void CTrafficFlowDlg::OnBnClickedBtnSunscirbeAndUnsubscribe()
{
	if (0 != m_lSubscribeTrafficFlowHandle)
	{
		CLIENT_StopLoadPic(m_lSubscribeTrafficFlowHandle);
		m_lSubscribeTrafficFlowHandle = 0;
		m_btnSubscribe.SetWindowText(ConvertString("Subscribe"));
		m_ctrTrafficFlow.DeleteAllItems();
	}
	else
	{ 
		int nChannel = m_cmbChannel.GetCurSel();
		m_nIndexOfSubscribeTrafficFlow = 0;
		m_lSubscribeTrafficFlowHandle = CLIENT_RealLoadPictureEx(m_lLoginHandle, 0, EVENT_IVS_TRAFFIC_FLOWSTATE, TRUE, cbTrafficFlowCallBack, (LDWORD)this, NULL);
		if (m_lSubscribeTrafficFlowHandle == 0)
		{		
			MessageBox(ConvertString("Subscribe intelligent failed!"), ConvertString("Prompt"));
			return;
		}
		else
		{
			m_btnSubscribe.SetWindowText(ConvertString("Unsubscribe"));
		}
	}
}

void CTrafficFlowDlg::OnCbnSelchangeCmbLane()
{
	m_nIndexOfSubscribeTrafficFlow = 0;
	m_ctrTrafficFlow.DeleteAllItems();
}

void CTrafficFlowDlg::OnBnClickedBtnQuery()
{
	// Whether the begin time is less than end time
	bool bRet = IsTimeCorrent();
	if (!bRet)
	{
		MessageBox(ConvertString("The begin time is bigger than end time, please input again!"), ConvertString("Prompt"));
		return;
	}

	CleanUpResultOfLastTimeQuery();

	// Start querying records
	m_lQueryTrafficFlowHandle = StartFindTrafficFlow();
	if (0 == m_lQueryTrafficFlowHandle)
	{
		return;
	}

	// Get total count of traffic flow
	m_nTotalCountOfTrafficFlow = FindTotalCountOfTrafficFlow();
	if (m_nTotalCountOfTrafficFlow <= 0)
	{
		return;
	}

	m_nCurPage = 1;
	SetDlgItemInt(IDC_EDIT_RECORD_COUNT, m_nTotalCountOfTrafficFlow);
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);

	int nQueryCount = SINGLE_QUERY_COUNT;
	if (m_nTotalCountOfTrafficFlow <= SINGLE_QUERY_COUNT)
	{
		nQueryCount = m_nTotalCountOfTrafficFlow ;
		m_nTotalPage = 1;
	}
	else
	{
		m_btnNextPage.EnableWindow(TRUE);
		m_nTotalPage = (int)ceil(m_nTotalCountOfTrafficFlow / (float)nQueryCount);
	}

	// Do find N number of traffic flow
	if (!DoFindTrafficFolw(nQueryCount))
	{
		m_btnNextPage.EnableWindow(FALSE);
		return;
	}

}

void CTrafficFlowDlg::CleanUpResultOfLastTimeQuery()
{
	m_nCurPage = 0;
	m_nTotalPage = 0;
	m_nTotalCountOfTrafficFlow = 0;
	m_nIndexOfQueryTrafficFlow = 0;

	m_btnNextPage.EnableWindow(FALSE);
	m_btnPreviousPage.EnableWindow(FALSE);
	m_ctrQueryTrafficFlow.DeleteAllItems();

	SetDlgItemText(IDC_EDIT_RECORD_COUNT, "");
	SetDlgItemText(IDC_EDIT_RECORD_CURRENT_PAGE, "");

	for (unsigned int i =0; i < m_vecTrafficFlow.size(); i++)
	{
		NET_RECORD_TRAFFIC_FLOW_STATE* pTrafficFlow = m_vecTrafficFlow[i];
		if (NULL != pTrafficFlow)
		{
			delete pTrafficFlow;
			pTrafficFlow = NULL;
		}
	}
	m_vecTrafficFlow.clear();

	if (0 != m_lSubscribeTrafficFlowHandle)
	{
		CLIENT_FindRecordClose(m_lSubscribeTrafficFlowHandle);
	}
}

LLONG CTrafficFlowDlg::StartFindTrafficFlow()
{
	FIND_RECORD_TRAFFICFLOW_CONDITION stTrafficFlow = {sizeof(FIND_RECORD_TRAFFICFLOW_CONDITION)};
	stTrafficFlow.abChannelId =TRUE;
	stTrafficFlow.nChannelId = 0;
	stTrafficFlow.abLane = FALSE;
	stTrafficFlow.bStartTime= TRUE;
	stTrafficFlow.bEndTime= TRUE;
	CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE);
	CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME);
	CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE);
	CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);
	GetTimeFromTimeCtr(stTrafficFlow.stStartTime, pCtrBeginDate, pCtrBeginTime);
	GetTimeFromTimeCtr(stTrafficFlow.stEndTime, pCtrEndtDate, pCtrEndTime);    
	stTrafficFlow.bStatisticsTime = TRUE;

	NET_IN_FIND_RECORD_PARAM stuFindInParam = {sizeof(NET_IN_FIND_RECORD_PARAM)};
	stuFindInParam.emType = NET_RECORD_TRAFFICFLOW_STATE;
	stuFindInParam.pQueryCondition = &stTrafficFlow;


	NET_OUT_FIND_RECORD_PARAM stuFindOutParam = {sizeof(NET_OUT_FIND_RECORD_PARAM)};
	BOOL bRet = CLIENT_FindRecord(m_lLoginHandle, &stuFindInParam, &stuFindOutParam, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Start query record failed!"), ConvertString("Prompt"));
		return 0;
	}
	return stuFindOutParam.lFindeHandle;
}


int CTrafficFlowDlg::FindTotalCountOfTrafficFlow()
{
	NET_IN_QUEYT_RECORD_COUNT_PARAM inQueryCountParam = { sizeof(NET_IN_QUEYT_RECORD_COUNT_PARAM)};
	inQueryCountParam.lFindeHandle =  m_lQueryTrafficFlowHandle;
	NET_OUT_QUEYT_RECORD_COUNT_PARAM outQueryCountParam  = { sizeof(NET_OUT_QUEYT_RECORD_COUNT_PARAM) };

	BOOL bRet = CLIENT_QueryRecordCount(&inQueryCountParam, &outQueryCountParam , MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Query record count failed!"), ConvertString("Prompt"));
		return -1;
	}
	if (0 == outQueryCountParam.nRecordCount)
	{
		MessageBox(ConvertString("Query zero!"), ConvertString("Prompt"));
		return 0;
	}
	return outQueryCountParam.nRecordCount;
}

bool CTrafficFlowDlg::DoFindTrafficFolw( int nQueryCount )
{
	NET_RECORD_TRAFFIC_FLOW_STATE* pRecordList = new NET_RECORD_TRAFFIC_FLOW_STATE[nQueryCount];
	memset(pRecordList, 0, sizeof(NET_RECORD_TRAFFIC_FLOW_STATE) * nQueryCount);
	for (int unIndex = 0; unIndex < nQueryCount; ++unIndex)
	{
		pRecordList[unIndex].dwSize = sizeof(NET_RECORD_TRAFFIC_FLOW_STATE);
	}

	NET_IN_FIND_NEXT_RECORD_PARAM stuFindNextInParam =  {sizeof(NET_IN_FIND_NEXT_RECORD_PARAM)};
	stuFindNextInParam.lFindeHandle = m_lQueryTrafficFlowHandle;
	stuFindNextInParam.nFileCount = nQueryCount;

	NET_OUT_FIND_NEXT_RECORD_PARAM stuFindNextOutParam = {sizeof(NET_OUT_FIND_NEXT_RECORD_PARAM)};
	stuFindNextOutParam.pRecordList = pRecordList;
	stuFindNextOutParam.nMaxRecordNum = nQueryCount;
	BOOL bRet = CLIENT_FindNextRecord(&stuFindNextInParam, &stuFindNextOutParam, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Query record failed!"), ConvertString("Prompt"));
		delete[] pRecordList;
		return false;
	}
	if (0 == stuFindNextOutParam.nRetRecordNum)
	{
		MessageBox(ConvertString("Query record over!"), ConvertString("Prompt"));
		delete[] pRecordList;
		return false;
	}

	for (int i = 0; i < stuFindNextOutParam.nRetRecordNum; i++)
	{ 
		m_nIndexOfQueryTrafficFlow++;
		ShowTrafficFlow(pRecordList+i, m_nIndexOfQueryTrafficFlow); 

		NET_RECORD_TRAFFIC_FLOW_STATE* pTrafficListInfo = new NET_RECORD_TRAFFIC_FLOW_STATE;
		memcpy(pTrafficListInfo, pRecordList+i, sizeof(NET_RECORD_TRAFFIC_FLOW_STATE));        
		m_vecTrafficFlow.push_back(pTrafficListInfo);
	}
	delete[] pRecordList;
	return true;
}

void CTrafficFlowDlg::ShowTrafficFlow(NET_RECORD_TRAFFIC_FLOW_STATE* pRecordList, int nCurrnetIndex)
{
	if (NULL == pRecordList)
	{
		return;
	}
	CString strIndex;
	CString strLane;
	CString strFlow;
	CString strHumanFlow;
	CString strAvgSpeed;
	CString strTimePercentage;
	CString strSpaceOccupyRate;

	strIndex.Format("%d", nCurrnetIndex);
	strLane.Format("%d", pRecordList->nLane);
	strFlow.Format("%d", pRecordList->nVehicles);
	strHumanFlow.Format("%d", pRecordList->nPasserby);
	strTimePercentage.Format("%.1f", pRecordList->fTimeOccupyRatio);
	strSpaceOccupyRate.Format("%.1f", pRecordList->fSpaceOccupyRatio);
	if (pRecordList->fAverageSpeed < 0)
	{
		strAvgSpeed.Format("-"); 
	}
	else
	{
		strAvgSpeed.Format("%.1f", pRecordList->fAverageSpeed);
	}

	int nCount =  m_ctrQueryTrafficFlow.GetItemCount();
	m_ctrQueryTrafficFlow.InsertItem(LVIF_TEXT|LVIF_STATE,nCount,strIndex,0,LVIS_SELECTED,0,0);
	m_ctrQueryTrafficFlow.SetItemText(nCount,0, strIndex);
	m_ctrQueryTrafficFlow.SetItemText(nCount,1, strLane);
	m_ctrQueryTrafficFlow.SetItemText(nCount,2, strFlow);
	m_ctrQueryTrafficFlow.SetItemText(nCount,3, strHumanFlow);
	m_ctrQueryTrafficFlow.SetItemText(nCount,4, strAvgSpeed);
	m_ctrQueryTrafficFlow.SetItemText(nCount,5, strTimePercentage);
	m_ctrQueryTrafficFlow.SetItemText(nCount,6, strSpaceOccupyRate);
}

void CTrafficFlowDlg::OnBnClickedBtnPreviousPage()
{
	if(1 >= m_nCurPage)
	{
		return;
	}

	m_ctrQueryTrafficFlow.DeleteAllItems();
	for (int i = 0; i < SINGLE_QUERY_COUNT; i++)
	{
		ShowTrafficFlow(m_vecTrafficFlow[(m_nCurPage-2)*SINGLE_QUERY_COUNT+i], (m_nCurPage-2)*SINGLE_QUERY_COUNT+i+1);
	}
	m_nCurPage--;
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);
	if (m_nCurPage <= 1)
	{
		m_btnPreviousPage.EnableWindow(FALSE);
	}

	m_btnNextPage.EnableWindow(TRUE);
}

void CTrafficFlowDlg::OnBnClickedBtnNextPage()
{
	if(0 >= m_nCurPage || m_nCurPage >= m_nTotalPage)
	{
		return;
	}
	m_ctrQueryTrafficFlow.DeleteAllItems();
	if (m_vecTrafficFlow.size() > m_nCurPage*SINGLE_QUERY_COUNT)
	{
		int nRemainCount = m_vecTrafficFlow.size() - m_nCurPage*SINGLE_QUERY_COUNT;
		int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT?SINGLE_QUERY_COUNT:nRemainCount;
		for (int i = 0; i < nQueryCount; i++)
		{
			ShowTrafficFlow(m_vecTrafficFlow[m_nCurPage*SINGLE_QUERY_COUNT+i], m_nCurPage*SINGLE_QUERY_COUNT+i+1);
		}
	}
	else
	{
		int nRemainCount = m_nTotalCountOfTrafficFlow - m_nCurPage*SINGLE_QUERY_COUNT;
		int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT?SINGLE_QUERY_COUNT:nRemainCount;
		if (!DoFindTrafficFolw(nQueryCount))
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

void CTrafficFlowDlg::GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime)
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



bool CTrafficFlowDlg::IsTimeCorrent()
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

void CTrafficFlowDlg::OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult)
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

void CTrafficFlowDlg::OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult)
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
