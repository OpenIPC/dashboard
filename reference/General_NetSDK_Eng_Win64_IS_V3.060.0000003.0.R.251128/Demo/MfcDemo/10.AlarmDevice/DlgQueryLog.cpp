// DlgQueryLog.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgQueryLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgQueryLog dialog


CDlgQueryLog::CDlgQueryLog(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */, NET_DEVICE_TYPE emDevType /* = NET_PRODUCT_NONE */, int nMaxPageSize /* = 10 */)
	: CDialog(CDlgQueryLog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgQueryLog)
	m_nPageLines	= 10;
	m_timeEnd		= 0;
	m_timeBegin		= 0;
	m_dataEnd		= COleDateTime::GetCurrentTime();
	m_dataBegin		= COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
	m_lLoginId		= lLoginId;
	m_nStartNo		= 0;
	m_nMaxPageSize	= nMaxPageSize;
	m_emDevType		= emDevType;
}

void CDlgQueryLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgQueryLog)
	DDX_Control(pDX, IDC_QUERYLOG_LIST_LOGLIST, m_logList);
	DDX_Control(pDX, IDC_QUERYLOG_COMBO_TYPE, m_cmbQueryType);
	DDX_Text(pDX, IDC_QUERYLOG_EDIT_PAGELINES, m_nPageLines);
//	DDV_MinMaxInt(pDX, m_nPageLines, 1, 1024);
	DDX_DateTimeCtrl(pDX, IDC_QUERYLOG_DATETIMEPICKER_ENDTIME, m_timeEnd);
	DDX_DateTimeCtrl(pDX, IDC_QUERYLOG_DATETIMEPICKER_STARTTIME, m_timeBegin);
	DDX_DateTimeCtrl(pDX, IDC_QUERYLOG_DATETIMEPICKER_ENDDAY, m_dataEnd);
	DDX_DateTimeCtrl(pDX, IDC_QUERYLOG_DATETIMEPICKER_STARTDAY, m_dataBegin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgQueryLog, CDialog)
	//{{AFX_MSG_MAP(CDlgQueryLog)
	ON_BN_CLICKED(IDC_QUERYLOG_BTN_QUERY, OnBtnQuery)
	ON_BN_CLICKED(IDC_QUERYLOG_BTN_PREPAGE, OnBtnPrePage)
	ON_BN_CLICKED(IDC_QUERYLOG_BTN_NEXTPAGE, OnBtnNextPage)
	ON_BN_CLICKED(IDC_QUERYLOG_BTN_CLRLOG, OnBtnClrLog)	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// private method

void CDlgQueryLog::InitDlg()
{
	m_logList.SetExtendedStyle(m_logList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_logList.InsertColumn(0, ConvertString("seq", DLG_QUERYLOG), LVCFMT_LEFT,30,-1);
	m_logList.InsertColumn(1, ConvertString("type", DLG_QUERYLOG), LVCFMT_LEFT,150,-1);
	m_logList.InsertColumn(2, ConvertString("date", DLG_QUERYLOG),LVCFMT_LEFT,180,-1);
	m_logList.InsertColumn(3, ConvertString("content", DLG_QUERYLOG),LVCFMT_LEFT,420,-1);

	m_cmbQueryType.ResetContent();
	int nIndex = m_cmbQueryType.AddString(ConvertString("all log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_ALL);
	nIndex = m_cmbQueryType.AddString(ConvertString("system log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_SYSTEM);
	nIndex = m_cmbQueryType.AddString(ConvertString("config log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_CONFIG);
	nIndex = m_cmbQueryType.AddString(ConvertString("storage log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_STORAGE);
	nIndex = m_cmbQueryType.AddString(ConvertString("alarm log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_ALARM);
	nIndex = m_cmbQueryType.AddString(ConvertString("record log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_RECORD);
	nIndex = m_cmbQueryType.AddString(ConvertString("accounts log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_ACCOUNT);
	nIndex = m_cmbQueryType.AddString(ConvertString("clean log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_CLEAR);
	nIndex = m_cmbQueryType.AddString(ConvertString("callback log", DLG_QUERYLOG));
	m_cmbQueryType.SetItemData(nIndex,DHLOG_PLAYBACK);
	m_cmbQueryType.SetCurSel(0);

	CTimeSpan timeSpan(1,0,0,0);
	CTime curTime = CTime::GetCurrentTime();
	CTime yesterday = curTime - timeSpan;
	m_dataBegin.SetDateTime(yesterday.GetYear(), yesterday.GetMonth(), yesterday.GetDay(), 
		yesterday.GetHour(), yesterday.GetMinute(), yesterday.GetSecond());
	m_timeBegin = yesterday;
	m_dataEnd.SetDateTime(curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());
	m_timeEnd = curTime;
	UpdateData(FALSE);

	if (NET_AF6016 == m_emDevType
		|| NET_AH2008 == m_emDevType)
	{
		// 该两款设备的日志查询功能，仅支持按照起始序号查询，故关闭不相关的控件
		m_cmbQueryType.EnableWindow(FALSE);
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_STARTDAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_ENDDAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_STARTTIME)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_ENDTIME)->EnableWindow(FALSE);
	}
	else // if (NET_AS5008 == m_emDevType)
	{
		// 即同普通DVR的日志查询功能
		m_cmbQueryType.EnableWindow();
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_STARTDAY)->EnableWindow();
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_ENDDAY)->EnableWindow();
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_STARTTIME)->EnableWindow();
		GetDlgItem(IDC_QUERYLOG_DATETIMEPICKER_ENDTIME)->EnableWindow();
	}
	
	GetDlgItem(IDC_QUERYLOG_BTN_NEXTPAGE)->EnableWindow(FALSE);
 	GetDlgItem(IDC_QUERYLOG_BTN_PREPAGE)->EnableWindow(FALSE);
}

int CDlgQueryLog::QueryLog(int nStartNo, int nPageSize)
{
	UpdateData(TRUE);
	if (nStartNo < 0 || nPageSize > 1024)
	{
		return -1;
	}
	
	if (NET_AF6016 == m_emDevType || NET_AH2008 == m_emDevType)
	{
		// 这些设备每次查询的最大日志条数较小
		if (m_nMaxPageSize < nPageSize)
		{
			MessageBox(ConvertString("Too large page size!", DLG_QUERYLOG), ConvertString("Prompt"));
			return -1;
		}
	}
	else
	{		
		if (m_dataBegin > m_dataEnd || (m_dataBegin == m_dataEnd && m_timeBegin > m_timeEnd))
		{
			MessageBox(ConvertString("Incorrect time!", DLG_QUERYLOG), ConvertString("Prompt"));
			return -1;
		}
	}
	
	QUERY_DEVICE_LOG_PARAM stuLogParam;
	memset(&stuLogParam, 0, sizeof(QUERY_DEVICE_LOG_PARAM));
	
	if (NET_AF6016 == m_emDevType
		|| NET_AH2008 == m_emDevType)
	{
		// 这两款设备仅仅支持"起始序号"这一查询条件
	}
	else
	{
		// 除了两款设备，支持如下的查询条件
		int nIndex				= m_cmbQueryType.GetCurSel();
		stuLogParam.emLogType	= (DH_LOG_QUERY_TYPE)m_cmbQueryType.GetItemData(nIndex);
		stuLogParam.nLogStuType = 1;						// 必须为1
	}
	
	stuLogParam.nStartNum		= nStartNo;					// 日志的起始序号，从0开始
	stuLogParam.nEndNum			= nStartNo + nPageSize -1;	// 本次查询的最后一条日志的序号
	
	stuLogParam.stuStartTime.dwYear		= m_dataBegin.GetYear();
	stuLogParam.stuStartTime.dwMonth	= m_dataBegin.GetMonth();
	stuLogParam.stuStartTime.dwDay		= m_dataBegin.GetDay();
	stuLogParam.stuStartTime.dwHour		= m_timeBegin.GetHour();
	stuLogParam.stuStartTime.dwMinute	= m_timeBegin.GetMinute();
	stuLogParam.stuStartTime.dwSecond	= m_timeBegin.GetSecond();
	
	stuLogParam.stuEndTime.dwYear		= m_dataEnd.GetYear();
	stuLogParam.stuEndTime.dwMonth		= m_dataEnd.GetMonth();
	stuLogParam.stuEndTime.dwDay		= m_dataEnd.GetDay();
	stuLogParam.stuEndTime.dwHour		= m_timeEnd.GetHour();
	stuLogParam.stuEndTime.dwMinute		= m_timeEnd.GetMinute();
	stuLogParam.stuEndTime.dwSecond		= m_timeEnd.GetSecond();
	
	DH_DEVICE_LOG_ITEM_EX* pstuLogInfo	= new DH_DEVICE_LOG_ITEM_EX[nPageSize];
	int nRetLogNum						= 0;

	BOOL bRet = CLIENT_QueryDeviceLog(m_lLoginId, &stuLogParam, (char*)pstuLogInfo, nPageSize * sizeof(DH_DEVICE_LOG_ITEM_EX), 
		&nRetLogNum);
	if (bRet)
	{
		if (nRetLogNum <= 0)
		{
			MessageBox(ConvertString("No more logs!", DLG_QUERYLOG), ConvertString("Prompt"));
		}
		else
		{
			m_logList.DeleteAllItems();

			for (int i = 0; i < nRetLogNum; i++)
			{
				const DHDEVTIME& stuTime = pstuLogInfo[i].stuOperateTime;
				
				CString strLogTime;
				strLogTime.Format("%04d-%02d-%02d %02d:%02d:%02d", 
					stuTime.year + 2000, stuTime.month, stuTime.day,
				stuTime.hour, stuTime.minute, stuTime.second);

				InsertListViewItem(nStartNo + i, CString(pstuLogInfo[i].szOperation), strLogTime, CString(pstuLogInfo[i].szDetailContext));
			}
		}
	}
	else
	{
		m_logList.DeleteAllItems();
		MessageBox(ConvertString("Query log failed!", DLG_QUERYLOG), ConvertString("Prompt"));
		GetDlgItem(IDC_QUERYLOG_BTN_NEXTPAGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUERYLOG_BTN_PREPAGE)->EnableWindow(FALSE);
	}
	
	delete[] pstuLogInfo;
	pstuLogInfo = NULL;

	return nRetLogNum;
}

void CDlgQueryLog::InsertListViewItem(int nStartNo, const CString& strLogType, const CString& strLogTime, const CString& strLogContext)
{
	int nIndex = m_logList.GetItemCount();
	
	CString strIndex;
	strIndex.Format("%d", nStartNo + 1);
	m_logList.InsertItem(nIndex, "");
	
	m_logList.SetItemText(nIndex, 0, strIndex);
	m_logList.SetItemText(nIndex, 1, strLogType);
	m_logList.SetItemText(nIndex, 2, strLogTime);
	m_logList.SetItemText(nIndex, 3, strLogContext);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgQueryLog message handlers

BOOL CDlgQueryLog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this, DLG_QUERYLOG);

	InitDlg();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//////////////////////////////////////////////////////////////////////////
//
// 初次查询，必须从第一条日志开始查询
//
//////////////////////////////////////////////////////////////////////////
void CDlgQueryLog::OnBtnQuery() 
{
	// TODO: Add your control notification handler code here
	m_nStartNo = 0;
	int nPageSize = GetDlgItemInt(IDC_QUERYLOG_EDIT_PAGELINES);
	int nRetNum = QueryLog(m_nStartNo, nPageSize);
	if (nRetNum == nPageSize)
	{
		// 表示可能还有更多日志
		GetDlgItem(IDC_QUERYLOG_BTN_NEXTPAGE)->EnableWindow();
		GetDlgItem(IDC_QUERYLOG_BTN_PREPAGE)->EnableWindow();
	}
	else // nRetNum < nPageSize
	{
		// 表示符合条件的日志通过本次查询全部获得，或者没有日志
		GetDlgItem(IDC_QUERYLOG_BTN_NEXTPAGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUERYLOG_BTN_PREPAGE)->EnableWindow(FALSE);
	}
}
//////////////////////////////////////////////////////////////////////////
//
// 前翻一页查询
//
//////////////////////////////////////////////////////////////////////////
void CDlgQueryLog::OnBtnPrePage() 
{
	// TODO: Add your control notification handler code here
	int nPageSize = GetDlgItemInt(IDC_QUERYLOG_EDIT_PAGELINES);
	if (nPageSize <= 0 || nPageSize > 1024)
	{
		MessageBox(ConvertString("Incorrect page size!", DLG_QUERYLOG), ConvertString("Prompt"));
		return;
	}

	if ((m_nStartNo - nPageSize) < 0)	// 已经前翻到第一页
	{
		m_nStartNo = 0;
		QueryLog(m_nStartNo, nPageSize);
	}
	else
	{
		m_nStartNo -= nPageSize;
		QueryLog(m_nStartNo, nPageSize);
	}
}
//////////////////////////////////////////////////////////////////////////
//
// 后翻一页查询
//
//////////////////////////////////////////////////////////////////////////
void CDlgQueryLog::OnBtnNextPage() 
{
	// TODO: Add your control notification handler code here
	int nPageSize = GetDlgItemInt(IDC_QUERYLOG_EDIT_PAGELINES);
	if (nPageSize <= 0 || nPageSize > 1024)
	{
		MessageBox(ConvertString("Incorrect page size!", DLG_QUERYLOG), ConvertString("Prompt"));
		return;
	}
	
	m_nStartNo += nPageSize;
	int nRet = QueryLog(m_nStartNo, nPageSize);
	if (nRet > 0)
	{
	}
	else // nRetNum <= 0
	{
		// 已经翻到最后一页，起始序号不应当再累加
		m_nStartNo -= nPageSize;
	}
}

void CDlgQueryLog::OnBtnClrLog() 
{
	BOOL bRet = CLIENT_ControlDevice(m_lLoginId, DH_CTRL_CLEARLOG, NULL, SDK_API_WAIT);
	if (bRet)
	{
		m_logList.DeleteAllItems();
		MessageBox(ConvertString("Clear log success!", DLG_QUERYLOG), ConvertString("Prompt"));
	}
	else
	{
        CString csInfo;
        csInfo.Format("%s:0x%08x", ConvertString("Clear log failed", DLG_QUERYLOG), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}
