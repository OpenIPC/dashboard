// LockRecord.cpp : implementation file
//

#include "stdafx.h"
#include "PlayBack.h"
#include "LockRecord.h"
#include "PlayBackDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLockRecord dialog
#define FILEDOWNLOAD_TIMER_MSEC 62.5

CLockRecord::CLockRecord(CWnd* pParent /*=NULL*/)
	: CDialog(CLockRecord::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLockRecord)
	m_timeFrom = 0;
	m_timeTo = 0;
	m_dateFrom = COleDateTime::GetCurrentTime();
	m_dateTo = COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
	m_lLoginID = 0;
	m_bUnLockQueryFlag = FALSE;
	m_nTimer = 0;
	m_dwTotalSize = 0;
	m_dwCurValue = 0;

	memset(m_netFileInfo,0,sizeof(m_netFileInfo));
}


void CLockRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLockRecord)
	DDX_Control(pDX, IDC_LIST_QUERYINFO, m_ctlListInfo);
	DDX_Control(pDX, IDC_COMBO_CHANNELBYFILE, m_ctlChannel);
	DDX_DateTimeCtrl(pDX, IDC_TIME_FROM, m_timeFrom);
	DDX_DateTimeCtrl(pDX, IDC_TIME_TO, m_timeTo);
	DDX_DateTimeCtrl(pDX, IDC_DATE_FROM, m_dateFrom);
	DDX_DateTimeCtrl(pDX, IDC_DATE_TO, m_dateTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLockRecord, CDialog)
	//{{AFX_MSG_MAP(CLockRecord)
	ON_BN_CLICKED(IDC_BUTTON_QUERY, OnButtonQuery)
	ON_BN_CLICKED(IDC_BUTTON_CLEANLIST, OnButtonCleanlist)
	ON_BN_CLICKED(IDC_BUTTON_LOCK, OnButtonLock)
	ON_BN_CLICKED(IDC_BUTTON_UNLOCK, OnButtonUnlock)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLockRecord message handlers

//Search
void CLockRecord::OnButtonQuery() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid && (0 != m_lLoginID))
	{
		//Recorded file information
		int nMaxLen = 2000 * sizeof(NET_RECORDFILE_INFO);
		int nFileCount = 0;

		//Recorded file tpye 
		int nRecordFileType = 0;

		//Channel number
		int nChannelId = 0;
		int nIndex = m_ctlChannel.GetCurSel();
		nChannelId = (int)m_ctlChannel.GetItemData(nIndex);
		//Time
		NET_TIME netTimeFrom = ConvertToDateTime(m_dateFrom,m_timeFrom);
		NET_TIME netTimeTo = ConvertToDateTime(m_dateTo,m_timeTo);
		
		//Search
		BOOL bSuccess = CLIENT_QueryRecordFile(m_lLoginID,nChannelId,nRecordFileType,
			&netTimeFrom,&netTimeTo,NULL,m_netFileInfo,nMaxLen,&nFileCount,5000,FALSE);
		if(bSuccess)
		{
			if(0 == nFileCount)
			{
				m_ctlListInfo.DeleteAllItems();
				MessageBox(ConvertString("NO locked record!"), ConvertString("Prompt"));
			}
			else
			{
				//Insert the searched results into ListView
				AddRecordInfoToListView(m_netFileInfo,nFileCount);
				if(m_ctlListInfo.GetItemCount() <=0&&!m_bUnLockQueryFlag)
				{
					MessageBox(ConvertString("NO locked record!"), ConvertString("Prompt"));
				}
				
			}
		}
		else
		{
            int nError = CLIENT_GetLastError();
            if (NET_NO_RECORD_FOUND == nError)
            {
				m_ctlListInfo.DeleteAllItems();
                MessageBox(ConvertString("NO locked record!"), ConvertString("Prompt"));
            }
            else
            {
                MessageBox(ConvertString("Query failed!"), ConvertString("Prompt"));
            }
		}
	}
	else
	{
		MessageBox(ConvertString("Please login first!"), ConvertString("Prompt"));
	}
}

//Clear list
void CLockRecord::OnButtonCleanlist() 
{
	// TODO: Add your control notification handler code here
	m_ctlListInfo.DeleteAllItems();
}

BOOL CLockRecord::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	InitListView();

	//Set current time 
	CTimeSpan timeSpan(1,0,0,0);
	CTime curTime = CTime::GetCurrentTime();
	CTime dateFrom = curTime - timeSpan;
	m_dateFrom.SetDateTime(dateFrom.GetYear(), dateFrom.GetMonth(), dateFrom.GetDay(), 
		dateFrom.GetHour(), dateFrom.GetMinute(), dateFrom.GetSecond());
	m_timeFrom = curTime;
	m_dateTo.SetDateTime(curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());
	m_timeTo = curTime;
	UpdateData(FALSE);

    if (0 == m_nTimer)
    {
        m_nTimer = SetTimer(1, FILEDOWNLOAD_TIMER_MSEC, NULL);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Set channel dropdown menu 
void CLockRecord::InitComboBox(int nChannel,LLONG lLoginID)
{
	m_lLoginID = lLoginID;
	m_ctlListInfo.DeleteAllItems();

    SetControlButton(TRUE);

	int nIndex = 0;
	CString str;
	m_ctlChannel.ResetContent();
	for(int i=0;i<nChannel;i++)
	{
		str.Format("%d",i+1);
		nIndex = m_ctlChannel.AddString(str);
		m_ctlChannel.SetItemData(nIndex,i);
	}
	m_ctlChannel.SetCurSel(0);
}

//ListView initialization
void CLockRecord::InitListView()
{
	m_ctlListInfo.SetExtendedStyle(m_ctlListInfo.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctlListInfo.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,50,-1);
	m_ctlListInfo.InsertColumn(1,ConvertString("Record Type"),LVCFMT_LEFT,60,0);
	m_ctlListInfo.InsertColumn(2,ConvertString("file size(KB)"),LVCFMT_LEFT,60,1);
	m_ctlListInfo.InsertColumn(3, ConvertString("Frame Num"), LVCFMT_LEFT,80,2);
	m_ctlListInfo.InsertColumn(4,ConvertString("Start time"),LVCFMT_LEFT,100,3);
	m_ctlListInfo.InsertColumn(5,ConvertString("End time"),LVCFMT_LEFT,100,4);
	m_ctlListInfo.InsertColumn(6,ConvertString("Disk No. "),LVCFMT_LEFT,50,5);
	m_ctlListInfo.InsertColumn(7,ConvertString("Lock file"),LVCFMT_LEFT,80,6);

}

//Turn time format CTime to NET_TIME type
NET_TIME CLockRecord::ConvertTime(CTime date,CTime time)
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

//Insert searched results into ListView
void CLockRecord::AddRecordInfoToListView(LPNET_RECORDFILE_INFO pRecordFiles, int nFileCount)
{
	m_ctlListInfo.DeleteAllItems();
    CString strRecordType[] = {ConvertString("CommonRecord") , ConvertString("AlarmRecord") , 
        ConvertString("MotionRecord") , ConvertString("CardRecord") , ConvertString("PicRecord") , 
        ConvertString("IntelligentRecord"), ConvertString("UnknowRecord")};
	for(int i=0;i<nFileCount;i++)
	{
		NET_RECORDFILE_INFO netFileInfo = pRecordFiles[i];
		if(netFileInfo.bImportantRecID)
		{
			CString strGetType;
			if( netFileInfo.nRecordFileType > 5)
			{
				strGetType = strRecordType[6];
			}
			else
			{
				strGetType = strRecordType[netFileInfo.nRecordFileType];
			}

			CString strNum;
			strNum.Format("%d",i);
			CString strFileName;
			strFileName.Format("%s",netFileInfo.filename);
			CString strFileLength;
			strFileLength.Format("%d",netFileInfo.size);
			CString strStartTime;
			strStartTime.Format(ConvertString("%dyear %dmonth %dday  %d:%d:%d"),netFileInfo.starttime.dwYear,
				netFileInfo.starttime.dwMonth,netFileInfo.starttime.dwDay,netFileInfo.starttime.dwHour,
				netFileInfo.starttime.dwMinute,netFileInfo.starttime.dwSecond);
			CString strEndTime;
			strEndTime.Format(ConvertString("%dyear %dmonth %dday  %d:%d:%d"),netFileInfo.endtime.dwYear,
				netFileInfo.endtime.dwMonth,netFileInfo.endtime.dwDay,netFileInfo.endtime.dwHour,
				netFileInfo.endtime.dwMinute,netFileInfo.endtime.dwSecond);
			CString strDisk;
			strDisk.Format("%d",netFileInfo.driveno);
			CString strLockfile;

			strLockfile = ConvertString("Yes");

			CString strFrameNum;
			strFrameNum.Format("%d", netFileInfo.framenum);

			//²åÈëÐÐ
			int nIndex = m_ctlListInfo.GetItemCount();
			m_ctlListInfo.InsertItem(LVIF_TEXT|LVIF_STATE,nIndex,strNum,0,LVIS_SELECTED,0,0);
			m_ctlListInfo.SetItemText(nIndex,1,strGetType);
			m_ctlListInfo.SetItemText(nIndex,2,strFileLength);
			m_ctlListInfo.SetItemText(nIndex,3,strFrameNum);
			m_ctlListInfo.SetItemText(nIndex,4,strStartTime);
			m_ctlListInfo.SetItemText(nIndex,5,strEndTime);
			m_ctlListInfo.SetItemText(nIndex,6,strDisk);
			m_ctlListInfo.SetItemText(nIndex,7,strLockfile);
		}
	}
}

#ifdef _WIN64
void CLockRecord::OnTimer(UINT_PTR nIDEvent)
#elif defined(_WIN32)
void CLockRecord::OnTimer(UINT nIDEvent)
#endif
{
	// TODO: Add your message handler code here and/or call default
	CDialog::OnTimer(nIDEvent);
}

void CLockRecord::SetControlButton(BOOL bFlag)
{
    GetDlgItem(IDC_COMBO_CHANNELBYFILE)->EnableWindow(bFlag);
    GetDlgItem(IDC_DATE_FROM)->EnableWindow(bFlag);
    GetDlgItem(IDC_TIME_FROM)->EnableWindow(bFlag);
    GetDlgItem(IDC_DATE_TO)->EnableWindow(bFlag);
    GetDlgItem(IDC_TIME_TO)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_QUERY)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_CLEANLIST)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_LOCK)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_UNLOCK)->EnableWindow(bFlag);
}

//Clear after log off 
void CLockRecord::Clean()
{
	m_dwTotalSize = 0;
	m_dwCurValue = 0;
    SetControlButton(FALSE);
}

NET_TIME CLockRecord::ConvertToDateTime(const COleDateTime &date, const CTime &time)
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

void CLockRecord::OnButtonLock() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid && (0 != m_lLoginID))
	{
		//Recorded file information
		int nMaxLen = 2000 * sizeof(NET_RECORDFILE_INFO);
		int nFileCount = 0;

		//Recorded file tpye 
		int nRecordFileType = 0;

		//Channel number
		int nChannelId = 0;
		int nIndex = m_ctlChannel.GetCurSel();
		nChannelId = (int)m_ctlChannel.GetItemData(nIndex);

		//Time
		NET_TIME netTimeFrom = ConvertToDateTime(m_dateFrom,m_timeFrom);
		NET_TIME netTimeTo = ConvertToDateTime(m_dateTo,m_timeTo);

		int nLockCount=0;
		//Search
		BOOL bSuccess = CLIENT_QueryRecordFile(m_lLoginID,nChannelId,nRecordFileType,
			&netTimeFrom,&netTimeTo,NULL,m_netFileInfo,nMaxLen,&nFileCount,5000,FALSE);
		if(bSuccess)
		{
			if(0 == nFileCount)
			{
				MessageBox(ConvertString("No RecordFile!"), ConvertString("Prompt"));
			}
			else
			{
				for(int i=0;i<nFileCount;i++)
				{
				    bSuccess = FALSE;
					m_netFileInfo[i].bImportantRecID = 1;
					bSuccess = CLIENT_ControlDevice(m_lLoginID,DH_MARK_IMPORTANT_RECORD,&m_netFileInfo[i]);
					if(bSuccess)
					{
						if(m_netFileInfo[i].bImportantRecID == 1)
						{
							nLockCount++;
						}
					}
				}
				if (nLockCount==nFileCount)
				{
					MessageBox(ConvertString("RecordFile lock success!"));
				}
				else
				{
                    MessageBox(ConvertString("RecordFile lock fail!"));
				}
			}
		}
		else
		{
			MessageBox(ConvertString("RecordFile lock fail!"));
		}
	}
	else
	{
		MessageBox(ConvertString("Please login first!"), ConvertString("Prompt"));
	}
}

void CLockRecord::OnButtonUnlock() 
{
	// TODO: Add your control notification handler code here
	if(m_lLoginID == 0)
	{
		MessageBox(ConvertString("Please login first!"), ConvertString("Prompt"));
		return;
	}
	int nIndex = m_ctlListInfo.GetNextItem(-1,LVNI_SELECTED);
	BOOL bSuccess = FALSE;
	if(nIndex != -1)
	{
		CString strNum=m_ctlListInfo.GetItemText(nIndex,0);
		int nNum=_ttoi(strNum);
		m_netFileInfo[nNum].bImportantRecID = 0;
		bSuccess = CLIENT_ControlDevice(m_lLoginID,DH_MARK_IMPORTANT_RECORD,&m_netFileInfo[nNum]);
		if(bSuccess)
		{
			m_bUnLockQueryFlag =TRUE;
			OnButtonQuery();
			m_bUnLockQueryFlag =FALSE;
			if(m_ctlListInfo.GetItemCount() >=0)
			{
				if(m_netFileInfo[nNum].bImportantRecID == 0)
				{
					MessageBox(ConvertString("File unlock success!"));
					return;
				}
			}
		}
		MessageBox(ConvertString("File unlock fail!"));
	}	
	else
	{
		MessageBox(ConvertString("Please choose the file first!"));
	}
}
