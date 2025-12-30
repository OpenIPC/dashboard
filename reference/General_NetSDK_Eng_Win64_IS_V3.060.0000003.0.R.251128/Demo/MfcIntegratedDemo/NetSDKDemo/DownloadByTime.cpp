// DownloadByTime.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "DownloadByTime.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDownloadByTime dialog


CDownloadByTime::CDownloadByTime(CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadByTime::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDownloadByTime)
	m_channelsel = 0;
	m_enddate = 0;
	m_endtime = 0;
	m_startdate = 0;
	m_starttime = 0;
	//}}AFX_DATA_INIT

	m_pdownloadfile = 0;
	m_downloadtotal = 0;
	m_downloadsize = 0;
	m_bInited = FALSE;

}


void CDownloadByTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDownloadByTime)
	DDX_Control(pDX, IDC_DLBT_DEVICESEL, m_devicesel);
	DDX_Control(pDX, IDC_DLBT_PROGRESS, m_download_pos);
	DDX_Text(pDX, IDC_DLBT_CHANNELSEL, m_channelsel);
	DDX_DateTimeCtrl(pDX, IDC_DLBT_ENDDATE, m_enddate);
	DDX_DateTimeCtrl(pDX, IDC_DLBT_ENDTIME, m_endtime);
	DDX_DateTimeCtrl(pDX, IDC_DLBT_STARTDATE, m_startdate);
	DDX_DateTimeCtrl(pDX, IDC_DLBT_STARTTIME, m_starttime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDownloadByTime, CDialog)
	//{{AFX_MSG_MAP(CDownloadByTime)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD, OnBtnDownload)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDownloadByTime message handlers

int InitComboxCallback(const DeviceNode& node, LDWORD dwUser)
{
	CDownloadByTime* pThis = (CDownloadByTime*)dwUser;
	if(!pThis)
	{
		return 1;
	}

	return pThis->InitComboxCallback_Imp(node);
}

int CDownloadByTime::InitComboxCallback_Imp(const DeviceNode& node)
{
	//设备显示
	CString strDev;	
	strDev.Format(" %s ",node.Name);
	int nIndex = m_devicesel.GetCount();
	m_devicesel.InsertString(nIndex,strDev);
	m_devicesel.SetItemData(nIndex, (DWORD)&node); //设置保存当前设备指针值

	return 0;
}

BOOL CDownloadByTime::OnInitDialog() 
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);
	
	CString strDev;	
	
	CDevMgr::GetDevMgr().For_EachDev(InitComboxCallback, (LDWORD)this);

	if (m_devicesel.GetCount() > 0) 
	{
		m_devicesel.SetCurSel(0);
	}
	
	CTime cur_time = CTime ::GetCurrentTime();
	CTime time = CTime(cur_time.GetYear(),cur_time.GetMonth(), cur_time.GetDay(),
		cur_time.GetHour(),cur_time.GetMinute(),cur_time.GetSecond());
	m_enddate = time;
	m_endtime = time;
	m_startdate = time;
	m_starttime = time;
	m_pdownloadfile = 0;
	
	m_channelsel = 1;
	
	UpdateData(FALSE);
	m_strIdx = "";
	m_strPos = "";
	m_strFileInfo = "";

	m_bInited = FALSE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CALLBACK BTDownLoadPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO recordfileinfo, LDWORD dwUser)
{
	CDownloadByTime * dlg = (CDownloadByTime *)dwUser;
	
	dlg->UpdateDownloadPos(lPlayHandle, dwTotalSize, dwDownLoadSize, index, recordfileinfo);
}

//更新下载进度
void CDownloadByTime::UpdateDownloadPos(LLONG iHandle, DWORD dwTotal, DWORD dwDownload, int index, NET_RECORDFILE_INFO& recordfileinfo)
{
	if(m_pdownloadfile != iHandle)
	{
		MessageBox(ConvertString(MSG_SCHRECORD_DOWNLORD_CB_ERR));
		return;
	}
	m_downloadtotal = dwTotal;
	m_downloadsize = dwDownload;

//	CString nstr;
//	nstr.Format("%d", index);
	//	GetDlgItem(IDC_EDIT_FILEINDEX)->SetWindowText(nstr);
	m_strIdx.Format("%d", index);
	m_strPos.Format("%d / %d",dwDownload, dwTotal);
	m_strFileInfo.Format(ConvertString("channel: %d, driver no: %d, size: %d\r\nstart time: %d-%d-%d-%d-%d-%d\r\nend time: %d-%d-%d-%d-%d-%d")
		,recordfileinfo.ch, recordfileinfo.driveno, recordfileinfo.size,
		recordfileinfo.starttime.dwYear,
		recordfileinfo.starttime.dwMonth,
		recordfileinfo.starttime.dwDay,
		recordfileinfo.starttime.dwHour,
		recordfileinfo.starttime.dwMinute,
		recordfileinfo.starttime.dwSecond,
		recordfileinfo.endtime.dwYear,
		recordfileinfo.endtime.dwMonth,
		recordfileinfo.endtime.dwDay,
		recordfileinfo.endtime.dwHour,
		recordfileinfo.endtime.dwMinute,
		recordfileinfo.endtime.dwSecond);
/*
	nstr.Format("%d / %d",dwDownload, dwTotal);
	GetDlgItem(IDC_EDIT_PROGRESS)->SetWindowText(nstr);

	nstr.Format("channel: %d, driver no: %d, size: %d\r\nstart time: %d-%d-%d-%d-%d-%d\r\nend time: %d-%d-%d-%d-%d-%d"
				,recordfileinfo.ch, recordfileinfo.driveno, recordfileinfo.size,
				recordfileinfo.starttime.dwYear,
				recordfileinfo.starttime.dwMonth,
				recordfileinfo.starttime.dwDay,
				recordfileinfo.starttime.dwHour,
				recordfileinfo.starttime.dwMinute,
				recordfileinfo.starttime.dwSecond,
				recordfileinfo.endtime.dwYear,
				recordfileinfo.endtime.dwMonth,
				recordfileinfo.endtime.dwDay,
				recordfileinfo.endtime.dwHour,
				recordfileinfo.endtime.dwMinute,
				recordfileinfo.endtime.dwSecond);
	GetDlgItem(IDC_EDIT_FILEINFO)->SetWindowText(nstr);*/
	if(-1 == m_downloadsize || m_downloadsize >= m_downloadtotal)
	{
		MessageBox(ConvertString(MSG_SCHRECORD_DOWNLOAD_FIN));
		if (m_pdownloadfile)
		{
			BOOL b = CLIENT_StopDownload(m_pdownloadfile);
			if (!b)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
			}
			m_pdownloadfile = NULL;	
		}
		((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD))->SetWindowText(ConvertString(MSG_SCHRECORD_DOWNLOAD));
	/**/}
	
}

void CDownloadByTime::OnBtnDownload() 
{
	if (!UpdateData(TRUE))
	{
		return;
	}
	if(m_devicesel.GetCount() <= 0 || m_devicesel.GetCurSel() < 0)
	{
		MessageBox(ConvertString(MSG_SCHRECORD_NODEVICE), ConvertString(MSG_SCHRECORD_CHOOSEDEVICE));
		return;
	}
	if(m_pdownloadfile)
	{
		BOOL stopret = CLIENT_StopDownload(m_pdownloadfile);
		if (!stopret) 
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
			MessageBox(ConvertString(MSG_SCHRECORD_STOPDLFAILED));
		}
		else
		{
			MessageBox(ConvertString(MSG_SCHRECORD_CANCELED));
			m_pdownloadfile = NULL;
			m_download_pos.SetPos(0);
			GetDlgItem(IDC_EDIT_PROGRESS)->SetWindowText("");
			GetDlgItem(IDC_EDIT_FILEINDEX)->SetWindowText("");
			GetDlgItem(IDC_EDIT_FILEINFO)->SetWindowText("");
			
			((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD))->SetWindowText(ConvertString(MSG_SCHRECORD_DOWNLOAD));
		}
		return;
	}
	
	SYSTEMTIME systime;
	NET_TIME stime, etime;
	CTime time = CTime(m_startdate.GetYear(),m_startdate.GetMonth(),m_startdate.GetDay(),
		m_starttime.GetHour(), m_starttime.GetMinute(), m_starttime.GetSecond());
	if(!time.GetAsSystemTime(systime) || !time.GetAsSystemTime(systime))
	{
		MessageBox(ConvertString(MSG_SCHRECORD_TIMEINPUT_ERR), ConvertString(MSG_SCHRECORD_TIMEINPUT));
		return;
	}
	g_systimetoprivatetime(&systime,&stime);
	
	time = CTime(m_enddate.GetYear(),m_enddate.GetMonth(),m_enddate.GetDay(),
		m_endtime.GetHour(), m_endtime.GetMinute(), m_endtime.GetSecond());
	if(!time.GetAsSystemTime(systime) || !time.GetAsSystemTime(systime))
	{
		MessageBox(ConvertString(MSG_SCHRECORD_TIMEINPUT_ERR), ConvertString(MSG_SCHRECORD_TIMEINPUT));
		return;
	}
	g_systimetoprivatetime(&systime,&etime);
	
	DeviceNode *pInfo = (DeviceNode *)m_devicesel.GetItemData(m_devicesel.GetCurSel());
	
	CString strName = "download.dav";
	
	CFileDialog dlg(FALSE,"*.dav","download.dav",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"All File(*.dav)|*.*||",this);
	if(dlg.DoModal() == IDOK)
	{
		strName = dlg.GetPathName();		
	}
	else
	{
		return;
	}
	
	m_pdownloadfile = CLIENT_DownloadByTime(pInfo->LoginID,
									m_channelsel - 1,
									0,
									&stime,
									&etime,
									strName.GetBuffer(0), 
									BTDownLoadPos, 
									(LDWORD)this);
	
	if (m_pdownloadfile)
	{
		((CButton *)GetDlgItem(IDC_BTN_DOWNLOAD))->SetWindowText(ConvertString(MSG_SCHRECORD_CANCELDOWNLOAD));
		SetTimer((LLONG)this, 1000,NULL);			
	}
	else
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		MessageBox(ConvertString(MSG_SCHRECORD_STARTDLFAILED));
	}
}

void CDownloadByTime::OnTimer(UINT_PTR nIDEvent) 
{
	if(m_pdownloadfile)
	{
		if(m_downloadtotal != 0)
		{
			m_download_pos.SetPos(m_downloadsize * 100/m_downloadtotal);
		}
		GetDlgItem(IDC_EDIT_FILEINDEX)->SetWindowText(m_strIdx);
		GetDlgItem(IDC_EDIT_PROGRESS)->SetWindowText(m_strPos);
		GetDlgItem(IDC_EDIT_FILEINFO)->SetWindowText(m_strFileInfo);
	}
	CDialog::OnTimer(nIDEvent);
}

void CDownloadByTime::OnClose() 
{
	if(m_pdownloadfile)
	{
		BOOL b = CLIENT_StopDownload(m_pdownloadfile);
		if (!b)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		m_pdownloadfile = NULL;
	}
	KillTimer((LLONG)this);
	CDialog::OnClose();
}

void CDownloadByTime::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow && !m_bInited)
	{
		CDevMgr::GetDevMgr().For_EachDev(InitComboxCallback, (LDWORD)this);
		if (m_devicesel.GetCount() > 0) 
		{
			m_devicesel.SetCurSel(0);
		}

		CTime cur_time = CTime ::GetCurrentTime();
		CTime time = CTime(cur_time.GetYear(),cur_time.GetMonth(), cur_time.GetDay(),
			cur_time.GetHour(),cur_time.GetMinute(),cur_time.GetSecond());
		m_enddate = time;
		m_endtime = time;
		m_startdate = time;
		m_starttime = time;
		m_pdownloadfile = 0;
		
		m_channelsel = 1;
		
		UpdateData(FALSE);

		m_bInited = TRUE;
	}
}
