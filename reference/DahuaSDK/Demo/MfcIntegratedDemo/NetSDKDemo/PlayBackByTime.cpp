// PlayBackByTime.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "PlayBackByTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayBackByTime dialog


CPlayBackByTime::CPlayBackByTime(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayBackByTime::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlayBackByTime)
	m_channelsel = 0;
	m_enddate = 0;
	m_endtime = 0;
	m_startdate = 0;
	m_starttime = 0;
	//}}AFX_DATA_INIT

	memset(&m_playbackinfo,0,sizeof(m_playbackinfo));

}


void CPlayBackByTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlayBackByTime)
	DDX_Control(pDX, IDC_SCH_DEVICESEL, m_devicesel);
	DDX_Text(pDX, IDC_SCH_CHANNELSEL, m_channelsel);
	DDX_DateTimeCtrl(pDX, IDC_SCH_ENDDATE, m_enddate);
	DDX_DateTimeCtrl(pDX, IDC_SCH_ENDTIME, m_endtime);
	DDX_DateTimeCtrl(pDX, IDC_SCH_STARTDATE, m_startdate);
	DDX_DateTimeCtrl(pDX, IDC_SCH_STARTTIME, m_starttime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlayBackByTime, CDialog)
	//{{AFX_MSG_MAP(CPlayBackByTime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayBackByTime message handlers

int InitComboxPbByTimeCallback(const DeviceNode& node, LDWORD dwUser)
{
	CPlayBackByTime* pThis = (CPlayBackByTime*)dwUser;
	if(!pThis)
	{
		return 1;
	}

	return pThis->InitComboxCallback_Imp(node);
}

int CPlayBackByTime::InitComboxCallback_Imp(const DeviceNode& node)
{
	//设备显示
	CString strDev;	
	strDev.Format(" %s ",node.Name);
	int nIndex = m_devicesel.GetCount();
	m_devicesel.InsertString(nIndex,strDev);
	m_devicesel.SetItemData(nIndex, (DWORD)(&node)); //设置保存当前设备指针值

	return 0;
}


BOOL CPlayBackByTime::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
		
	m_devicesel.Clear();	

	CDevMgr::GetDevMgr().For_EachDev(InitComboxPbByTimeCallback, (LDWORD)this);

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

	m_channelsel = 1;

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlayBackByTime::OnOK() 
{
	DeviceNode *pInfo;
	SYSTEMTIME systime;
	NET_TIME stime, etime;
	
	if(!UpdateData(true))
	{
		return;
	}
	if(m_devicesel.GetCount() <= 0 || m_devicesel.GetCurSel() < 0)
	{
		MessageBox(ConvertString(MSG_SCHRECORD_NODEVICE), ConvertString(MSG_SCHRECORD_CHOOSEDEVICE));
		return;
	}
	if(m_channelsel <= 0/* || m_channelsel > DH_MAX_CHANNUM*/)
	{
		MessageBox(ConvertString(MSG_SCHRECORD_CHLNO_INVALID), ConvertString(MSG_SCHRECORD_CHLNO));	
		return;
	}

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

	pInfo = (DeviceNode *)m_devicesel.GetItemData(m_devicesel.GetCurSel());
	
	m_playbackinfo.pDevice = pInfo;
	m_playbackinfo.nChannel = m_channelsel - 1;
	m_playbackinfo.starttime = stime;
	m_playbackinfo.endtime = etime;

	CDialog::OnOK();
}

PlayBackByTimeInfo* CPlayBackByTime::GetPlayBackInfo()
{
	return &m_playbackinfo;
}
