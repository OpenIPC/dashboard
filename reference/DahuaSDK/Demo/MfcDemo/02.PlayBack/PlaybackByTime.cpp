// PlaybackByTime.cpp : implementation file
//

#include "stdafx.h"
#include "PlayBack.h"
#include "PlaybackByTime.h"
#include "PlayBackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlaybackByTime dialog
#define DOWNBYTIME_TIMER_SECOND 62.5

CPlaybackByTime::CPlaybackByTime(CWnd* pParent /*=NULL*/)
	: CDialog(CPlaybackByTime::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlaybackByTime)
	m_timeLoadFrom = 0;
	m_timeLoadTo = 0;
	m_dateLoadFrom = COleDateTime::GetCurrentTime();
	m_dateLoadTo = COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
	m_hLoginID = 0;
	m_dwTotalSize = 0;
	m_dwCurValue = 0;
	m_nTimer = 0;
	m_nPlayDirection = -1;

	memset(m_netFileInfo,0,sizeof(m_netFileInfo));
}


void CPlaybackByTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaybackByTime)
	DDX_Control(pDX, IDC_COMBO_STREAMTYPE, m_ctlStreamType);
	DDX_Control(pDX, IDC_COMBO_QUERYTYPE, m_ctlQueryType);
	DDX_Control(pDX, IDC_COMBO_TIMEPLAYMODE, m_ctlPlayMode);
	DDX_Control(pDX, IDC_COMBO_LOADCHANNEL, m_ctlLoadChannel);
	DDX_Control(pDX, IDC_PROGRESS_BYTIME, m_ctlLoadPro);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_ctlChannel);
	DDX_Control(pDX, IDC_COMBO_PLAYDIRECTION, m_ctlPlayDirection);
	DDX_DateTimeCtrl(pDX, IDC_TIME_LOAD_FROM, m_timeLoadFrom);
	DDX_DateTimeCtrl(pDX, IDC_TIME_LOAD_TO, m_timeLoadTo);
	DDX_DateTimeCtrl(pDX, IDC_DATE_LOAD_FROM, m_dateLoadFrom);
	DDX_DateTimeCtrl(pDX, IDC_DATE_LOAD_TO, m_dateLoadTo);
	DDX_Control(pDX, IDC_COMBO_LOADCHANNEL2, m_comboConvertType);

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlaybackByTime, CDialog)
	//{{AFX_MSG_MAP(CPlaybackByTime)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, OnButtonDownload)
	ON_WM_TIMER()
	ON_NOTIFY(MCN_SELECT, IDC_MONTHCALENDAR, OnSelectMonthcalendar)
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYDIRECTION, &CPlaybackByTime::OnCbnSelchangeComboPlayReverse)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD2, &CPlaybackByTime::OnBnClickedButtonDownload2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaybackByTime message handlers

//Playback 
void CPlaybackByTime::OnButtonPlay(int nPlayDirection) 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);

    // Set search type first
    int nStreamIndex = m_ctlStreamType.GetCurSel();
    DWORD dwStream = m_ctlStreamType.GetItemData(nStreamIndex);
    BOOL bRet = CLIENT_SetDeviceMode(m_hLoginID, DH_RECORD_STREAM_TYPE, &dwStream);
    if (TRUE != bRet)
    {
        MessageBox(ConvertString("Set stream type failed"), ConvertString("Prompt"));
    }

	int nRecordFileType = 0;
	int nIndex = m_ctlQueryType.GetCurSel();	
	DWORD dwItemData = m_ctlQueryType.GetItemData(nIndex);

	if (EM_LISTALL == dwItemData)		//Search all recorded file
	{
		nRecordFileType = 0;
	}
	else if(EM_ALARMLOCAL == dwItemData)	//Search alarmlocal record 
	{
		nRecordFileType = 1;
	}
	else if(EM_VIDEOMOTION == dwItemData)	//Search alarmlocal record 
	{
		nRecordFileType = 2;
	}

	if(bValid)
	{
		//Channel number 
		int nChannelId = 0;
		int nIndex = m_ctlChannel.GetCurSel();
		nChannelId = (int)m_ctlChannel.GetItemData(nIndex);
		nIndex = m_ctlPlayMode.GetCurSel();
		if(CB_ERR != nIndex)
		{
			PlayBackMode ePlayMode = (PlayBackMode)m_ctlPlayMode.GetItemData(nIndex);
			if(ePlayMode == EM_SERVERMODE)
			{
				((CPlayBackDlg *)AfxGetMainWnd())->PlayBackByTimeServerMode(nChannelId, nPlayDirection, nRecordFileType);
			}
			else
			{
				((CPlayBackDlg *)AfxGetMainWnd())->PlayBackByTime(nChannelId, nPlayDirection, nRecordFileType);
			}
		}
	}
}

//Download 
void CPlaybackByTime::OnButtonDownload() 
{
	// TODO: Add your control notification handler code here
	CString strText;
	GetDlgItemText(IDC_BUTTON_DOWNLOAD,strText);
	if(strText == "Download" || strText == ConvertString("Download") )
	{
		BOOL bValid = UpdateData(TRUE);
		if(bValid)
		{
			//Channel number
			int nIndex = m_ctlLoadChannel.GetCurSel();
			int nChannelId = (int)m_ctlLoadChannel.GetItemData(nIndex);
			//Time
			NET_TIME netTimeFrom = ConvertToDateTime(m_dateLoadFrom,m_timeLoadFrom);
			NET_TIME netTimeTo = ConvertToDateTime(m_dateLoadTo,m_timeLoadTo);
			if (Compare(&netTimeFrom, &netTimeTo) >= 0)
			{
				MessageBox(ConvertString("Invalid time!"), ConvertString("Prompt"));
				return;
			}
			BOOL bSuccess = ((CPlayBackDlg *)AfxGetMainWnd())->DownLoadByTime(nChannelId,&netTimeFrom,&netTimeTo,TRUE);
			if(bSuccess)
			{
				SetDlgItemText(IDC_BUTTON_DOWNLOAD, ConvertString("Stop"));
			}
		}
	}
	else
	{
		NET_TIME netTimeFrom = {0};
		NET_TIME netTimeTo = {0};
		BOOL bSuccess = ((CPlayBackDlg *)AfxGetMainWnd())->DownLoadByTime(0,&netTimeFrom,&netTimeTo,FALSE);
		if(bSuccess)
		{
			SetDlgItemText(IDC_BUTTON_DOWNLOAD, ConvertString("Download"));
			m_dwTotalSize = 0;
			m_dwCurValue = 0;
			long hDownLoad = ((CPlayBackDlg *)AfxGetMainWnd())->m_pDownLoadByTime;
			if(0 != hDownLoad)
			{
				CLIENT_StopDownload(hDownLoad);
				((CPlayBackDlg *)AfxGetMainWnd())->m_pDownLoadByTime = 0;
			}
			m_ctlLoadPro.SetRange32(0,0);
			m_ctlLoadPro.SetPos(0);
			Invalidate();
		}
	}
}

void CPlaybackByTime::SetControlButton(BOOL bFlag)
{
    GetDlgItem(IDC_COMBO_CHANNEL)->EnableWindow(bFlag);
    GetDlgItem(IDC_COMBO_TIMEPLAYMODE)->EnableWindow(bFlag);
    GetDlgItem(IDC_COMBO_QUERYTYPE)->EnableWindow(bFlag);
    GetDlgItem(IDC_MONTHCALENDAR)->EnableWindow(bFlag);
    GetDlgItem(IDC_COMBO_LOADCHANNEL)->EnableWindow(bFlag);
	GetDlgItem(IDC_COMBO_LOADCHANNEL2)->EnableWindow(bFlag);

    GetDlgItem(IDC_DATE_LOAD_FROM)->EnableWindow(bFlag);
    GetDlgItem(IDC_TIME_LOAD_FROM)->EnableWindow(bFlag);
    GetDlgItem(IDC_DATE_LOAD_TO)->EnableWindow(bFlag);
    GetDlgItem(IDC_TIME_LOAD_TO)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_DOWNLOAD)->EnableWindow(bFlag);
    GetDlgItem(IDC_COMBO_STREAMTYPE)->EnableWindow(bFlag);
	GetDlgItem(IDC_COMBO_PLAYDIRECTION)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_DOWNLOAD2)->EnableWindow(bFlag);

}

//Set channel dropdown menu 
void CPlaybackByTime::InitComboBox(int nChannel,LLONG lLoginID)
{
	m_hLoginID = lLoginID;

    SetControlButton(TRUE);
	int nIndex = 0;
	int i = 0;
	CString str;
	m_ctlChannel.ResetContent();
	for(i=0;i<nChannel;i++)
	{
		str.Format("%d",i+1);
		nIndex = m_ctlChannel.AddString(str);
		m_ctlChannel.SetItemData(nIndex,i);
	}
	m_ctlChannel.SetCurSel(0);

	m_ctlLoadChannel.ResetContent();
	for(i=0;i<nChannel;i++)
	{
		str.Format("%d",i+1);
		nIndex = m_ctlLoadChannel.AddString(str);
		m_ctlLoadChannel.SetItemData(nIndex,i);
	}
	m_ctlLoadChannel.SetCurSel(0);

	CString strConvertType[15]={ConvertString("PRIVATE_Type"), ConvertString("GBPS_Type"),ConvertString("TS_TYPE"),
	ConvertString("MP4_TYPE"),ConvertString("H264_TYPE"),ConvertString("FLV_TYPE"),ConvertString("PS_TYPE"),
	ConvertString("DHTS_TYPE"),ConvertString("CDJFPS_TYPE"),ConvertString("RTP_TYPE")};
	m_comboConvertType.ResetContent();
	for(int i = 0;i<10;i++)
	{
		nIndex = m_comboConvertType.AddString(strConvertType[i]);
		m_comboConvertType.SetItemData(nIndex,i);
	}
	m_comboConvertType.SetCurSel(0);
}

BOOL CPlaybackByTime::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	int nIndex = m_ctlQueryType.AddString(ConvertString("All"));
	m_ctlQueryType.SetItemData(nIndex,EM_LISTALL);
	nIndex = m_ctlQueryType.AddString(ConvertString("AlarmLocal"));
	m_ctlQueryType.SetItemData(nIndex,EM_ALARMLOCAL);
	nIndex = m_ctlQueryType.AddString(ConvertString("VideoMotion"));
	m_ctlQueryType.SetItemData(nIndex,EM_VIDEOMOTION);
	m_ctlQueryType.SetCurSel(0);

    m_ctlStreamType.ResetContent();
    nIndex = m_ctlStreamType.AddString(ConvertString("AllStream"));
    m_ctlStreamType.SetItemData(nIndex,EM_BOTH_STREAM);
    nIndex = m_ctlStreamType.AddString(ConvertString("MainStream"));
    m_ctlStreamType.SetItemData(nIndex,EM_MAIN_STREAM);
    nIndex = m_ctlStreamType.AddString(ConvertString("ExtraStream"));
    m_ctlStreamType.SetItemData(nIndex,EM_EXTRA_STREAM);
	m_ctlStreamType.SetCurSel(EM_BOTH_STREAM);

	// TODO: Add extra initialization here
	//Set current time
	CTimeSpan timeSpan(1,0,0,0);
	CTime curTime = CTime::GetCurrentTime();
	CTime dateFrom = curTime - timeSpan;
	CTime dateLoadFrom = curTime - timeSpan;
	m_dateLoadFrom.SetDateTime(dateLoadFrom.GetYear(), dateLoadFrom.GetMonth(), dateLoadFrom.GetDay(), 
		dateLoadFrom.GetHour(), dateLoadFrom.GetMinute(), dateLoadFrom.GetSecond());
	m_timeLoadFrom = curTime;
	m_dateLoadTo.SetDateTime(curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());
	m_timeLoadTo = curTime;

	m_ctlPlayMode.ResetContent();
    nIndex = m_ctlPlayMode.AddString( ConvertString("direct"));
	m_ctlPlayMode.SetItemData(nIndex,EM_DIRECTMODE);
	nIndex = m_ctlPlayMode.AddString(ConvertString("callback"));
	m_ctlPlayMode.SetItemData(nIndex,EM_SERVERMODE);
	m_ctlPlayMode.SetCurSel(0);

	m_ctlPlayDirection.ResetContent();
	nIndex = m_ctlPlayDirection.AddString( ConvertString("Forward"));
	m_ctlPlayDirection.SetItemData(nIndex,EM_FORWARD);
	nIndex = m_ctlPlayDirection.AddString(ConvertString("Backward"));
	m_ctlPlayDirection.SetItemData(nIndex,EM_BACKWARD);
	m_ctlPlayDirection.SetCurSel(EM_FORWARD);

	m_ctlLoadPro.SetRange32(0,(int)(m_dwTotalSize/100));
	m_ctlLoadPro.SetPos(0);
    if (0 == m_nTimer)
    {
	    m_nTimer = SetTimer(1, DOWNBYTIME_TIMER_SECOND, NULL);
    }
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Turn time format CTime to NET_TIME type
NET_TIME CPlaybackByTime::ConvertTime(CTime date, CTime time)
{
	NET_TIME netTime = {0};
	netTime.dwYear = date.GetYear();
	netTime.dwMonth = date.GetMonth();
	netTime.dwDay = date.GetDay();
	netTime.dwHour = time.GetHour();
	netTime.dwMinute = time.GetMinute();
	netTime.dwSecond =time.GetSecond();
	return netTime;
}

#ifdef _WIN64
void CPlaybackByTime::OnTimer(UINT_PTR nIDEvent)
#elif defined(_WIN32)
void CPlaybackByTime::OnTimer(UINT nIDEvent)
#endif 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == m_nTimer)
	{
		m_ctlLoadPro.SetRange32(0,(int)(m_dwTotalSize/100));
		m_ctlLoadPro.SetPos((int)(m_dwCurValue/100));
	}
	
	CDialog::OnTimer(nIDEvent);
}

//Logg off and then clear 
void CPlaybackByTime::Clean()
{
	SetDlgItemText(IDC_BUTTON_DOWNLOAD, ConvertString("Download"));
	SetDlgItemText(IDC_BUTTON_DOWNLOAD2, ConvertString("DownloadByType"));
	m_dwTotalSize = 0;
	m_dwCurValue = 0;
    SetControlButton(FALSE);
}

NET_TIME CPlaybackByTime::ConvertToDateTime(const COleDateTime &date, const CTime &time)
{
	NET_TIME netTime = {0};
	int year = date.GetYear();
	if (year < 2000)
	{
		netTime.dwYear = 2000;
		netTime.dwMonth = 1;
		netTime.dwDay = 1;
		netTime.dwHour = 0;
		netTime.dwMinute = 0;
		netTime.dwSecond = 0;
	}
	else
	{
		netTime.dwYear = date.GetYear();
		netTime.dwMonth = date.GetMonth();
		netTime.dwDay = date.GetDay();
		netTime.dwHour = time.GetHour();
		netTime.dwMinute = time.GetMinute();
		netTime.dwSecond =time.GetSecond();
	}
	
	return netTime;
}

/////////////////////////////////////////////////////////////////////////////////
/// \fn int CPlaybackByTime::Compare( const NET_TIME *pFristTime, const NET_TIME *pLastTime )
/// \brief 比较两时间段的大小
/// \param[in] pSrcTime 比较的源时间
/// \param[in] pDestTime  比较的目的时间
/// \exception 无
/// \return -1 第一个时间小； 0 两时间相等； 1 第一个时间大
/////////////////////////////////////////////////////////////////////////////////
int CPlaybackByTime::Compare( const NET_TIME *pSrcTime, const NET_TIME *pDestTime)
{
	SYSTEMTIME srcTime={0}, destTime={0};
	srcTime.wYear   = (WORD)pSrcTime->dwYear;
	srcTime.wMonth  = (WORD)pSrcTime->dwMonth;
	srcTime.wDay    = (WORD)pSrcTime->dwDay;
	srcTime.wHour   = (WORD)pSrcTime->dwHour;
	srcTime.wMinute = (WORD)pSrcTime->dwMinute;
	srcTime.wSecond = (WORD)pSrcTime->dwSecond;
    /// dest
	destTime.wYear  = (WORD)pDestTime->dwYear;
	destTime.wMonth = (WORD)pDestTime->dwMonth;
	destTime.wDay   = (WORD)pDestTime->dwDay;
	destTime.wHour  = (WORD)pDestTime->dwHour;
	destTime.wMinute = (WORD)pDestTime->dwMinute;
	destTime.wSecond = (WORD)pDestTime->dwSecond;
	FILETIME srcFileTime={0}, destFileTime={0};
	SystemTimeToFileTime(&srcTime, &srcFileTime);
	SystemTimeToFileTime(&destTime, &destFileTime);
	return CompareFileTime(&srcFileTime, &destFileTime);
}

void CPlaybackByTime::OnSelectMonthcalendar(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid && (0 != m_hLoginID))
	{
		//Recorded file information
		int nMaxLen = 2000 * sizeof(NET_RECORDFILE_INFO);
		int nFileCount = 1;
		//Recorded file tpye 
		int nRecordFileType = 0;
		int nIndex = m_ctlQueryType.GetCurSel();	
		DWORD dwItemData = m_ctlQueryType.GetItemData(nIndex);
		
		// Set search type first
        int nStreamIndex = m_ctlStreamType.GetCurSel();
        DWORD dwStream = m_ctlStreamType.GetItemData(nStreamIndex);
        BOOL bRet = CLIENT_SetDeviceMode(m_hLoginID, DH_RECORD_STREAM_TYPE, &dwStream);
        if (TRUE != bRet)
        {
            MessageBox(ConvertString("Set stream type failed"), ConvertString("Prompt"));
        }

		if (EM_LISTALL == dwItemData)		//Search all recorded file
		{
			nRecordFileType = 0;
		}
		else if(EM_ALARMLOCAL == dwItemData)	//Search alarmlocal record 
		{
			nRecordFileType = 1;
		}
		else if(EM_VIDEOMOTION == dwItemData)	//Search VIDEOMOTION record 
		{
			nRecordFileType = 2;
		}
		
		//PlayBack Mode
		nIndex = m_ctlPlayMode.GetCurSel();
		if(CB_ERR != nIndex)
		{
			PlayBackMode ePlayMode = (PlayBackMode)m_ctlPlayMode.GetItemData(nIndex);
			if(ePlayMode == EM_SERVERMODE)
			{
				((CPlayBackDlg *)AfxGetMainWnd())->m_bPlayBackByTimeMode = 1;
			}
			else
			{
				((CPlayBackDlg *)AfxGetMainWnd())->m_bPlayBackByTimeMode = 0;
			}
		}
		//Channel number
		int nChannelId = 0;
		nIndex = m_ctlChannel.GetCurSel();
		nChannelId = (int)m_ctlChannel.GetItemData(nIndex);
		//Time
		SYSTEMTIME sysTime;
		CMonthCalCtrl * pMonthCalendar = (CMonthCalCtrl*)GetDlgItem(IDC_MONTHCALENDAR);
		pMonthCalendar->GetCurSel(&sysTime);   // 获取选中项的日期

		//Time
		NET_TIME netTimeFrom;
		NET_TIME netTimeTo;
		
		netTimeFrom.dwYear = sysTime.wYear;
		netTimeFrom.dwMonth = sysTime.wMonth;
		netTimeFrom.dwDay = sysTime.wDay;
		netTimeFrom.dwHour = 0;
		netTimeFrom.dwMinute = 0;
		netTimeFrom.dwSecond = 0;
		
		netTimeTo.dwYear = sysTime.wYear;
		netTimeTo.dwMonth = sysTime.wMonth;
		netTimeTo.dwDay = sysTime.wDay;
		netTimeTo.dwHour = 23;
		netTimeTo.dwMinute = 59;
		netTimeTo.dwSecond = 59;

		if (-1 == m_nPlayDirection)
		{
			int nPlayDirectionIndex = m_ctlPlayDirection.GetCurSel();
			DWORD dwPlayDirection = m_ctlPlayDirection.GetItemData(nPlayDirectionIndex);
			if (EM_FORWARD == dwPlayDirection) // 正放
			{
				m_nPlayDirection = 0;
			}
			else if (EM_BACKWARD == dwPlayDirection) // 倒放
			{
				m_nPlayDirection = 1;
			}
		}

		//Search
		DWORD dwFindHandle = CLIENT_FindFile(m_hLoginID, nChannelId, nRecordFileType, NULL, &netTimeFrom, &netTimeTo, FALSE, 10000);
		if (dwFindHandle != 0)
		{
			int nRet = 1;
			while(nRet > 0)
			{
				nRet = CLIENT_FindNextFile(dwFindHandle, &m_netFileInfo[nFileCount -1]);
				if (nRet > 0)
				{ 
                    if (nFileCount >= sizeof(m_netFileInfo)/sizeof(m_netFileInfo[0]))
                    {
						CLIENT_FindClose(dwFindHandle);
                        MessageBox(ConvertString("Too many recordfiles!"), ConvertString("Prompt"));
				        return;
                    }
					nFileCount++;
				}
			}
			CLIENT_FindClose(dwFindHandle);
		}
		else 
		{
			nFileCount = 0;
		}

        if(nFileCount > 0)
		{
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeStart.dwYear = sysTime.wYear;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeStart.dwMonth = sysTime.wMonth;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeStart.dwDay = sysTime.wDay;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeStart.dwHour = 0;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeStart.dwMinute = 0;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeStart.dwSecond = 0;

			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeEnd.dwYear = sysTime.wYear;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeEnd.dwMonth = sysTime.wMonth;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeEnd.dwDay = sysTime.wDay;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeEnd.dwHour = 23;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeEnd.dwMinute = 59;
			((CPlayBackDlg *)AfxGetMainWnd())->m_PlayBackByTimeEnd.dwSecond = 59;

			((CPlayBackDlg *)AfxGetMainWnd())->AddTimeToDateTimeCtl(m_netFileInfo, nFileCount, m_nPlayDirection);
		}
		else
		{
            if (0 == dwFindHandle)
            {
                int nError = CLIENT_GetLastError();
                if (NET_NO_RECORD_FOUND == nError)
                {
                    MessageBox(ConvertString("NO record!"), ConvertString("Prompt"));
                }
                else
                {
                    MessageBox(ConvertString("Query failed!"), ConvertString("Prompt"));
                }
            }
			else
			{
				((CPlayBackDlg *)AfxGetMainWnd())->StopPlay();
				MessageBox(ConvertString("NO record!"), ConvertString("Prompt"));
			}
		}
	}
	*pResult = 0;
}

void CPlaybackByTime::OnCbnSelchangeComboPlayReverse()
{
	// TODO: Add your control notification handler code here
	int nPlayDirectionIndex = m_ctlPlayDirection.GetCurSel();
	DWORD dwPlayDirection = m_ctlPlayDirection.GetItemData(nPlayDirectionIndex);
	TC_ITEM ti;
	ti.mask = TCIF_TEXT;
	char *pcText = NULL;
	char szText[32] = {0};
	if (EM_FORWARD == dwPlayDirection) // 正放
	{
		m_nPlayDirection = 0;
		sprintf(szText, ConvertString("Playback by time"));
		ti.pszText = szText;
		((CPlayBackDlg *)AfxGetMainWnd())->m_Tab.SetItem(0, &ti);
	}
	else if (EM_BACKWARD == dwPlayDirection) // 倒放
	{
		m_nPlayDirection = 1;
		sprintf(szText, ConvertString("Playback reverse by time"));
		ti.pszText = szText;
		((CPlayBackDlg *)AfxGetMainWnd())->m_Tab.SetItem(0, &ti);
	}

	((CPlayBackDlg *)AfxGetMainWnd())->m_Tab.SetMinTabWidth(-1);
	((CPlayBackDlg *)AfxGetMainWnd())->m_Tab.SetPadding(CSize(4,4));
}

void CPlaybackByTime::OnBnClickedButtonDownload2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strText;
	GetDlgItemText(IDC_BUTTON_DOWNLOAD2,strText);
	if(strText == "DownloadByType" || strText == ConvertString("DownloadByType") )
	{
		BOOL bValid = UpdateData(TRUE);
		if(bValid)
		{
			//Channel number
			int nIndex = m_ctlLoadChannel.GetCurSel();
			int nChannelId = (int)m_ctlLoadChannel.GetItemData(nIndex);
			//Convert Type
			nIndex = m_comboConvertType.GetCurSel();
			int ConvertType = (int)m_comboConvertType.GetItemData(nIndex);
			//Time
			NET_TIME netTimeFrom = ConvertToDateTime(m_dateLoadFrom,m_timeLoadFrom);
			NET_TIME netTimeTo = ConvertToDateTime(m_dateLoadTo,m_timeLoadTo);
			if (Compare(&netTimeFrom, &netTimeTo) >= 0)
			{
				MessageBox(ConvertString("Invalid time!"), ConvertString("Prompt"));
				return;
			}
			BOOL bSuccess = ((CPlayBackDlg *)AfxGetMainWnd())->DownLoadByType(nChannelId,&netTimeFrom,&netTimeTo,TRUE,ConvertType);
			if(bSuccess)
			{
				SetDlgItemText(IDC_BUTTON_DOWNLOAD2, ConvertString("Stop"));
			}
		}
	}
	else
	{
		NET_TIME netTimeFrom = {0};
		NET_TIME netTimeTo = {0};
		BOOL bSuccess = ((CPlayBackDlg *)AfxGetMainWnd())->DownLoadByType(0,&netTimeFrom,&netTimeTo,FALSE,0);
		if(bSuccess)
		{
			SetDlgItemText(IDC_BUTTON_DOWNLOAD2, ConvertString("DownloadByType"));
			m_dwTotalSize = 0;
			m_dwCurValue = 0;
			long hDownLoad = ((CPlayBackDlg *)AfxGetMainWnd())->m_pDownLoadByTime;
			if(0 != hDownLoad)
			{
				CLIENT_StopDownload(hDownLoad);
				((CPlayBackDlg *)AfxGetMainWnd())->m_pDownLoadByTime = 0;
			}
			m_ctlLoadPro.SetRange32(0,0);
			m_ctlLoadPro.SetPos(0);
			Invalidate();
		}
	}
}

