// HistoryQuery.cpp : 实现文件
//

#include "stdafx.h"
#include "PassengerFlow.h"
#include "HistoryQuery.h"


// CHistoryQuery 对话框
#define PER_PAGE_NUM 10
#define SEARCH_COUNT_NUM 11

IMPLEMENT_DYNAMIC(CHistoryQuery, CDialog)

CHistoryQuery::CHistoryQuery(LLONG lLogin, int nChannel, CWnd* pParent /*=NULL*/)
	: CDialog(CHistoryQuery::IDD, pParent)
	, m_lLoginHandle(lLogin)
	, m_nChannel(nChannel)
	, m_lQueryHandle(0)
	, m_nPage(0)
	, m_nRuleType(0)
	, m_nStayTime(0)
	, m_QueryCount(0)
{
	memset(&m_BeginTime, 0, sizeof(m_BeginTime) );
	memset(&m_EndTime, 0, sizeof(m_EndTime) );
}

CHistoryQuery::~CHistoryQuery()
{
}

void CHistoryQuery::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_QUERY_RESULT, m_lcQueryInfo);
	DDX_Control(pDX, IDC_COMBO_QUERY_TYPE, m_cbxType);
	DDX_Control(pDX, IDC_COMBO_QUERY_CHANNEL, m_cbxQueryChannel);
	DDX_Control(pDX, IDC_COMBO_RULE_TYPE, m_cbxRuleType);
}


BEGIN_MESSAGE_MAP(CHistoryQuery, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_QUERY, &CHistoryQuery::OnBnClickedButtonQuery)
	ON_CBN_SELCHANGE(IDC_COMBO_QUERY_TYPE, &CHistoryQuery::OnCbnSelchangeComboQueryType)
	ON_CBN_SELCHANGE(IDC_COMBO_RULE_TYPE, &CHistoryQuery::OnCbnSelchangeComboRuleType)
	ON_BN_CLICKED(IDC_BUTTON_NEXTPAGE, &CHistoryQuery::OnBnClickedButtonNextpage)
	ON_BN_CLICKED(IDC_BUTTON_PREPAGE, &CHistoryQuery::OnBnClickedButtonPrepage)
END_MESSAGE_MAP()


// CHistoryQuery 消息处理程序

BOOL CHistoryQuery::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	InitControl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CHistoryQuery::OnBnClickedButtonQuery()
{
	StopFindNumberState();
	m_lcQueryInfo.DeleteAllItems();

	GetDlgItem(IDC_BUTTON_PREPAGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_NEXTPAGE)->EnableWindow(FALSE);

	m_nPage = 0;
	m_QueryCount = 0;
	m_lcQueryInfo.DeleteAllItems();
	int nSel = m_cbxQueryChannel.GetCurSel();
	if (nSel == CB_ERR)
	{
		return ;
	}
	int nChannel = m_cbxQueryChannel.GetItemData(nSel);

	int nType = m_cbxType.GetCurSel();
	if (nType == CB_ERR)
	{
		return ;
	}

	GetTimeFromUI(nType);
	BOOL bRet = StartFindNumberState(nChannel, nType);
	if (bRet && m_QueryCount > 0)
	{
		int nCount = DoFindNumberStarteEx(0, SEARCH_COUNT_NUM);
		if (nCount == SEARCH_COUNT_NUM)
		{
			GetDlgItem(IDC_BUTTON_NEXTPAGE)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_PREPAGE)->EnableWindow(FALSE);
		}
	}
}

void CHistoryQuery::InitControl()
{
	m_lcQueryInfo.SetExtendedStyle(m_lcQueryInfo.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lcQueryInfo.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,60,-1);
	m_lcQueryInfo.InsertColumn(1,ConvertString("Channel"),LVCFMT_LEFT,60,0);
	m_lcQueryInfo.InsertColumn(2,ConvertString("RuleName"),LVCFMT_LEFT,100,1);
	m_lcQueryInfo.InsertColumn(3,ConvertString("UTC"),LVCFMT_LEFT,230,2);
	m_lcQueryInfo.InsertColumn(4,ConvertString("Info"),LVCFMT_LEFT,200,3);
	//m_lcQueryInfo.InsertColumn(5, ConvertString("Exited total"), LVCFMT_LEFT,60,4);

	m_cbxType.ResetContent();
	m_cbxType.AddString(ConvertString("Daily list"));
	m_cbxType.AddString(ConvertString("Monthly list"));
	m_cbxType.AddString(ConvertString("Yearly list"));
	m_cbxType.SetCurSel(0);

	m_cbxRuleType.ResetContent();
	m_cbxRuleType.AddString(ConvertString("NumberStat"));
	m_cbxRuleType.AddString(ConvertString("ManNumDetection"));
	m_cbxRuleType.SetCurSel(0);

	m_cbxQueryChannel.ResetContent();
	for(int i=0;i<m_nChannel;i++)
	{
		CString str;
		str.Format("%d",i + 1);
		m_cbxQueryChannel.AddString(str);
		m_cbxQueryChannel.SetItemData(i, (DWORD_PTR)i);
	}
	m_cbxQueryChannel.SetCurSel(0);
}

BOOL CHistoryQuery::StartFindNumberState(int nChannel, int nType)
{
	m_nStayTime = 0;
	BOOL bRet = FALSE;
	NET_IN_FINDNUMBERSTAT stInParam;
	memset(&stInParam, 0, sizeof(NET_IN_FINDNUMBERSTAT));

	stInParam.dwSize = sizeof(NET_IN_FINDNUMBERSTAT);
	stInParam.nChannelID = nChannel;
	stInParam.stStartTime.dwYear = m_BeginTime.nYear;
	stInParam.stStartTime.dwMonth = m_BeginTime.nMonth;
	stInParam.stStartTime.dwDay = m_BeginTime.nDay;
	stInParam.stStartTime.dwHour = m_BeginTime.nHour;

	stInParam.stEndTime.dwYear = m_EndTime.nYear;
	stInParam.stEndTime.dwMonth = m_EndTime.nMonth;
	stInParam.stEndTime.dwDay = m_EndTime.nDay;
	stInParam.stEndTime.dwHour = m_EndTime.nHour;

	// valid time: 2000-01-01 ~ 2037-12-31
	if (stInParam.stStartTime.dwYear < 2000 || 
		stInParam.stEndTime.dwYear > 2037)
	{
		MessageBox(ConvertString("Date input illegal valid time: 2000-01-01 ~ 2037-12-31"), ConvertString("Prompt"));
		return FALSE;
	}

	//NumberStat
	if (m_nRuleType == 1)
	{
		m_nStayTime = GetDlgItemInt(IDC_EDIT_RETENTION_TIME);
		stInParam.nMinStayTime = m_nStayTime;
		stInParam.emRuleType = EM_RULE_MAN_NUM_DETECTION;
	}
	//ManNumDetection
	else
	{
		stInParam.emRuleType = EM_RULE_NUMBER_STAT;
	}

	if (nType == 0)
	{
		stInParam.nGranularityType = 1;		//1:hour
	}
	else if (nType == 1)
	{
		stInParam.nGranularityType = 2;		//2:day
		stInParam.stEndTime.dwHour = 23;
		stInParam.stEndTime.dwMinute = 59;
		stInParam.stEndTime.dwSecond = 59;
	}
	else if (nType == 2)
	{
		stInParam.nGranularityType = 4;		//4:month
		stInParam.stEndTime.dwHour = 23;
		stInParam.stEndTime.dwMinute = 59;
		stInParam.stEndTime.dwSecond = 59;
	}
	stInParam.nWaittime = DEFAULT_WAIT_TIME;

	NET_OUT_FINDNUMBERSTAT pstOutParam ;
	memset(&pstOutParam, 0, sizeof(NET_OUT_FINDNUMBERSTAT));
	pstOutParam.dwSize = sizeof(NET_OUT_FINDNUMBERSTAT);

	m_lQueryHandle = CLIENT_StartFindNumberStat(m_lLoginHandle, &stInParam, &pstOutParam);
	if (m_lLoginHandle)
	{
		m_QueryCount = pstOutParam.dwTotalCount;
		bRet = TRUE;
	}

	return bRet;
}

int CHistoryQuery::DoFindNumberStarteEx(int nBegin, int nSearchCount)
{
	int nRetCount = -1;

	NET_IN_DOFINDNUMBERSTAT stInDoFind = {sizeof(stInDoFind)};
	NET_OUT_DOFINDNUMBERSTAT stOutDoFind;
	memset(&stOutDoFind,0, sizeof(stOutDoFind) );
	stOutDoFind.dwSize= sizeof(stOutDoFind);

	stInDoFind.nBeginNumber = nBegin;
	stInDoFind.nCount = nSearchCount;
	stInDoFind.nWaittime = DEFAULT_WAIT_TIME;
	stOutDoFind.pstuNumberStat = NEW DH_NUMBERSTAT[nSearchCount];
	if (NULL == stOutDoFind.pstuNumberStat)
	{
		return nRetCount;
	}
	memset(stOutDoFind.pstuNumberStat, 0, sizeof(DH_NUMBERSTAT) * nSearchCount);
	stOutDoFind.nBufferLen = sizeof(DH_NUMBERSTAT) * nSearchCount;
	for (unsigned int i = 0; i < nSearchCount; i++)
	{
		stOutDoFind.pstuNumberStat[i].dwSize = sizeof(DH_NUMBERSTAT);
	}

	int nRet = CLIENT_DoFindNumberStat(m_lQueryHandle, &stInDoFind, &stOutDoFind);
	if (nRet)
	{
		nRetCount = stOutDoFind.nCount;
		int nUpdateInfoCount = nRetCount;
		if(nRetCount == SEARCH_COUNT_NUM)
		{
			nUpdateInfoCount = PER_PAGE_NUM;
		}
		UpdateLcQueryInfo(stOutDoFind.pstuNumberStat, nUpdateInfoCount);
	}

	if (stOutDoFind.pstuNumberStat)
	{
		delete[] stOutDoFind.pstuNumberStat;
		stOutDoFind.pstuNumberStat = NULL;
	}

	return nRetCount;
}

void CHistoryQuery::UpdateLcQueryInfo(DH_NUMBERSTAT *pstuNumberStat, int nCount)
{
	m_lcQueryInfo.DeleteAllItems();
	CString strIndex;
	CString strChannel;
	CString strRuleName;
	CString strTime;
	CString strInfo;
	for (int n = 0; n < nCount; n++)
	{
		strIndex.Format("%d", n + m_nPage * PER_PAGE_NUM + 1);
		strChannel.Format("%d", pstuNumberStat[n].nChannelID + 1);
		strRuleName.Format("%s", pstuNumberStat[n].szRuleName);
		strTime.Format("%d-%02d-%02d %02d:%02d:%02d---%d-%02d-%02d %02d:%02d:%02d",
			pstuNumberStat[n].stuStartTime.dwYear,
			pstuNumberStat[n].stuStartTime.dwMonth,
			pstuNumberStat[n].stuStartTime.dwDay,
			pstuNumberStat[n].stuStartTime.dwHour,
			pstuNumberStat[n].stuStartTime.dwMinute,
			pstuNumberStat[n].stuStartTime.dwSecond,
			pstuNumberStat[n].stuEndTime.dwYear,
			pstuNumberStat[n].stuEndTime.dwMonth,
			pstuNumberStat[n].stuEndTime.dwDay,
			pstuNumberStat[n].stuEndTime.dwHour,
			pstuNumberStat[n].stuEndTime.dwMinute,
			pstuNumberStat[n].stuEndTime.dwSecond);

		//人数统计
		if (m_nRuleType == 0)
		{
			strInfo.Format("%s:%d   %s:%d", 
							ConvertString("Enter"),
							pstuNumberStat[n].nEnteredSubTotal,
							ConvertString("Exit"),
							pstuNumberStat[n].nExitedSubtotal);
		}
		//区域内人数统计
		else if (m_nRuleType == 1)
		{
			strInfo.Format("%s>=%dS :%d",
						ConvertString("Stranding Time"),
						m_nStayTime,
						pstuNumberStat[n].nInsideSubtotal);
		}
		
		m_lcQueryInfo.InsertItem(n, strIndex);
		m_lcQueryInfo.SetItemText(n, 1, strChannel);
		m_lcQueryInfo.SetItemText(n, 2, ConvertString(strRuleName) );
		m_lcQueryInfo.SetItemText(n, 3, strTime);
		m_lcQueryInfo.SetItemText(n, 4, strInfo);
	}
}

void CHistoryQuery::StopFindNumberState()
{
	if (m_lQueryHandle)
	{
		BOOL bRet = CLIENT_StopFindNumberStat(m_lQueryHandle);
		if (bRet == FALSE)
		{
			MessageBox(ConvertString("Stop Query fail!"), ConvertString("Prompt"));
		}
	}
	m_lQueryHandle = 0;
}
void CHistoryQuery::OnCbnSelchangeComboQueryType()
{
	int nIndex = m_cbxType.GetCurSel();
	if (nIndex == CB_ERR)
	{
		return;
	}
	else if (nIndex == 0)
	{
		//SetDlgItemText(IDC_STATIC_PS, ConvertString("(The maximun query range is 24 hours!)"));
		GetDlgItem(IDC_TIME_BEGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_TIME_END)->EnableWindow(TRUE);
	}
	else if (nIndex == 1)
	{
		//SetDlgItemText(IDC_STATIC_PS, ConvertString("(The maximun query range is one month!)"));
		GetDlgItem(IDC_TIME_BEGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIME_END)->EnableWindow(FALSE);
	}
	else if (nIndex == 2)
	{
		//SetDlgItemText(IDC_STATIC_PS, ConvertString("(The maximun query range is 12 months!)"));
		GetDlgItem(IDC_TIME_BEGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIME_END)->EnableWindow(FALSE);
	}
}

void CHistoryQuery::GetTimeFromUI(int nType)
{
	CDateTimeCtrl *pCDataBegin = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_BEGIN);
	CDateTimeCtrl *pCTimeBegin = (CDateTimeCtrl*)GetDlgItem(IDC_TIME_BEGIN);
	CDateTimeCtrl *pCDataEnd = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_END);
	CDateTimeCtrl *pCTimeEnd = (CDateTimeCtrl*)GetDlgItem(IDC_TIME_END);

	SYSTEMTIME DataBegin = {0};
	SYSTEMTIME TimeBegin = {0};
	SYSTEMTIME DataEnd = {0};
	SYSTEMTIME TimeEnd = {0};

	pCDataBegin->GetTime(&DataBegin);
	pCTimeBegin->GetTime(&TimeBegin);
	pCDataEnd->GetTime(&DataEnd);
	pCTimeEnd->GetTime(&TimeEnd);

	m_BeginTime.nYear = DataBegin.wYear;
	m_BeginTime.nMonth = DataBegin.wMonth;
	m_BeginTime.nDay = DataBegin.wDay;
	
	m_EndTime.nYear = DataEnd.wYear;
	m_EndTime.nMonth = DataEnd.wMonth;
	m_EndTime.nDay = DataEnd.wDay;

	if (nType == 0)
	{
		m_BeginTime.nHour = TimeBegin.wHour;
		m_EndTime.nHour = TimeEnd.wHour;
	}
	else
	{
		m_BeginTime.nHour = 0;
		m_EndTime.nHour = 0;
	}
}
void CHistoryQuery::OnCbnSelchangeComboRuleType()
{
	int nSel = m_cbxRuleType.GetCurSel();
	if (CB_ERR == nSel)
	{
		return;
	}
	else if (nSel == 0)
	{
		GetDlgItem(IDC_EDIT_RETENTION_TIME)->EnableWindow(FALSE);
		SetDlgItemText(IDC_EDIT_RETENTION_TIME, "");
		m_nRuleType = 0;
	}
	else if(nSel == 1)
	{
		GetDlgItem(IDC_EDIT_RETENTION_TIME)->EnableWindow(TRUE);
		SetDlgItemText(IDC_EDIT_RETENTION_TIME, "30");
		m_nRuleType = 1;
	}
}

void CHistoryQuery::OnBnClickedButtonNextpage()
{
	m_nPage++;
	GetDlgItem(IDC_BUTTON_PREPAGE)->EnableWindow(TRUE);
	int nSearchCount = DoFindNumberStarteEx(m_nPage * PER_PAGE_NUM, SEARCH_COUNT_NUM);
	if (nSearchCount == SEARCH_COUNT_NUM)
	{
		GetDlgItem(IDC_BUTTON_NEXTPAGE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_NEXTPAGE)->EnableWindow(FALSE);
	}
}


void CHistoryQuery::OnBnClickedButtonPrepage()
{
	m_nPage--;
	int nCount = DoFindNumberStarteEx(m_nPage * PER_PAGE_NUM, SEARCH_COUNT_NUM);
	if (m_nPage == 0)
	{
		GetDlgItem(IDC_BUTTON_PREPAGE)->EnableWindow(FALSE);
	}
	GetDlgItem(IDC_BUTTON_NEXTPAGE)->EnableWindow(TRUE);
}

BOOL CHistoryQuery::PreTranslateMessage(MSG* pMsg)
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
