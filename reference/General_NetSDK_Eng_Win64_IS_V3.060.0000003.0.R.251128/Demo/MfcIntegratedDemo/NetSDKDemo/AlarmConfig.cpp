// AlarmConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "AlarmConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlarmConfig dialog


CAlarmConfig::CAlarmConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAlarmConfig)
	m_bAlarmIn = FALSE;
	m_alarmname = _T("");
	m_hour1 = 0;
	m_hour2 = 0;
	m_hour3 = 0;
	m_hour4 = 0;
	m_minute1 = 0;
	m_minute2 = 0;
	m_minute3 = 0;
	m_minute4 = 0;
	m_shour1 = 0;
	m_shour2 = 0;
	m_shour3 = 0;
	m_shour4 = 0;
	m_sminute1 = 0;
	m_sminute3 = 0;
	m_sminute4 = 0;
	m_sminute2 = 0;
	//}}AFX_DATA_INIT
}


void CAlarmConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlarmConfig)
	DDX_Control(pDX, IDC_COPYOUTSEL, m_copyoutsel);
	DDX_Control(pDX, IDC_COPYALARMINSEL, m_copyalarminsel);
	DDX_Control(pDX, IDC_COPYALARMDAYSEL, m_copyalarmdaysel);
	DDX_Control(pDX, IDC_ALARMTYPESEL, m_alarmtypesel);
	DDX_Control(pDX, IDC_ALARMOUTTIMESEL, m_alarmouttimesel);
	DDX_Control(pDX, IDC_ALARMOUTSEL, m_alarmoutsel);
	DDX_Control(pDX, IDC_ALARMOUTDAYSEL, m_alarmdaysel);
	DDX_Control(pDX, IDC_ALARMINSEL, m_alarminsel);
	DDX_Check(pDX, IDC_ALARMIN_CHECK, m_bAlarmIn);
	DDX_Text(pDX, IDC_ALARMNAME, m_alarmname);
	DDX_Text(pDX, IDC_HOUR1, m_hour1);
	DDX_Text(pDX, IDC_HOUR2, m_hour2);
	DDX_Text(pDX, IDC_HOUR3, m_hour3);
	DDX_Text(pDX, IDC_HOUR4, m_hour4);
	DDX_Text(pDX, IDC_MINUTE1, m_minute1);
	DDX_Text(pDX, IDC_MINUTE2, m_minute2);
	DDX_Text(pDX, IDC_MINUTE3, m_minute3);
	DDX_Text(pDX, IDC_MINUTE4, m_minute4);
	DDX_Text(pDX, IDC_SHOUR1, m_shour1);
	DDX_Text(pDX, IDC_SHOUR2, m_shour2);
	DDX_Text(pDX, IDC_SHOUR3, m_shour3);
	DDX_Text(pDX, IDC_SHOUR4, m_shour4);
	DDX_Text(pDX, IDC_SMINUTE1, m_sminute1);
	DDX_Text(pDX, IDC_SMINUTE3, m_sminute3);
	DDX_Text(pDX, IDC_SMINUTE4, m_sminute4);
	DDX_Text(pDX, IDC_SMINUTE2, m_sminute2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlarmConfig, CDialog)
	//{{AFX_MSG_MAP(CAlarmConfig)
	ON_BN_CLICKED(IDC_ALARMAPPLY, OnAlarmapply)
	ON_CBN_SELCHANGE(IDC_ALARMINSEL, OnSelchangeAlarminsel)
	ON_BN_CLICKED(IDC_ALARMOUTRESTORE, OnAlarmoutrestore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlarmConfig message handlers

void CAlarmConfig::SetDevice(DeviceNode *nDev)
{
	m_Device = nDev;
}

BOOL CAlarmConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);

	//界面初始化
	m_alarmouttimesel.InsertString(0, MSG_ALARMCFG_5SEC);
	m_alarmouttimesel.InsertString(1, MSG_ALARMCFG_10SEC);
	m_alarmouttimesel.InsertString(2, MSG_ALARMCFG_30SEC);
	m_alarmouttimesel.InsertString(3, MSG_ALARMCFG_1MIN);
	m_alarmouttimesel.InsertString(4, MSG_ALARMCFG_2MIN);
	m_alarmouttimesel.InsertString(5, MSG_ALARMCFG_5MIN);
	m_alarmouttimesel.InsertString(6, MSG_ALARMCFG_10MIN);
	m_alarmouttimesel.InsertString(7, MSG_ALARMCFG_MANUAL);

	m_alarmtypesel.InsertString(0, MSG_ALARMCFG_UOPEN);
	m_alarmtypesel.InsertString(1, MSG_ALARMCFG_UCLOSE);

	m_alarmdaysel.InsertString(0 ,ConvertString(MSG_ALARMCFG_WHOLEWEEK));
	m_alarmdaysel.InsertString(1 ,ConvertString(MSG_ALARMCFG_MONDAY));
	m_alarmdaysel.InsertString(2 ,ConvertString(MSG_ALARMCFG_TUESDAY));
	m_alarmdaysel.InsertString(3 ,ConvertString(MSG_ALARMCFG_WEDNESDAY));
	m_alarmdaysel.InsertString(4 ,ConvertString(MSG_ALARMCFG_THURSDAY));
	m_alarmdaysel.InsertString(5 ,ConvertString(MSG_ALARMCFG_FRIDAY));
	m_alarmdaysel.InsertString(6 ,ConvertString(MSG_ALARMCFG_SATURDAY));
	m_alarmdaysel.InsertString(7 ,ConvertString(MSG_ALARMCFG_SUNDAY));

	m_copyalarmdaysel.InsertString(0 ,ConvertString(MSG_ALARMCFG_WHOLEWEEK));
	m_copyalarmdaysel.InsertString(1 ,ConvertString(MSG_ALARMCFG_MONDAY));
	m_copyalarmdaysel.InsertString(2 ,ConvertString(MSG_ALARMCFG_TUESDAY));
	m_copyalarmdaysel.InsertString(3 ,ConvertString(MSG_ALARMCFG_WEDNESDAY));
	m_copyalarmdaysel.InsertString(4 ,ConvertString(MSG_ALARMCFG_THURSDAY));
	m_copyalarmdaysel.InsertString(5 ,ConvertString(MSG_ALARMCFG_FRIDAY));
	m_copyalarmdaysel.InsertString(6 ,ConvertString(MSG_ALARMCFG_SATURDAY));
	m_copyalarmdaysel.InsertString(7 ,ConvertString(MSG_ALARMCFG_SUNDAY));


	bSetAlarmParams = FALSE;
	CString sTemp;
	char cTemp[100];
	int i;
	
	m_alarmin = 0;
//	m_nCopyAlarmIn = -1;
	m_alarmout = 0;
//	m_nCopyAlarmOut = -1;
//	m_nCopyOutTime = -1;
	m_alarminsel.ResetContent();
	m_copyalarminsel.ResetContent();
	m_alarmoutsel.ResetContent();
	m_copyoutsel.ResetContent();
	sTemp.Format(ConvertString(_T(NAME_ALARMCFG_ALLINPUT)));
	m_copyalarminsel.AddString(sTemp);
	for(i = 0; i < m_Device->Info.byAlarmInPortNum; i++)
	{	
	//	if(m_bConnectServer)
	//	{
			if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_ALARMINCFG, i,&m_alarmincfg[i], sizeof(NET_DEV_ALARMINCFG), &dwReturned, CONFIG_WAITTIME))
			{
				m_bGetAlarmIn[i] = FALSE;
			//	sTemp.Format("%d 报警参数获取失败: NET_DVR_GET_ALARMINCFG = %d \n", i,NET_DVR_GetLastError());
				AfxMessageBox(ConvertString(MSG_ALARMCFG_GETINPUTCFGFAILED));
			}
			else
			{
				m_bGetAlarmIn[i] = TRUE;
				sTemp.Format("%d", (1+i));
				sTemp = NAME_ALARMCFG_INPUT + sTemp;
				m_alarminsel.AddString(sTemp);
				m_copyalarminsel.AddString(sTemp);
			}
	//	}
	//	else
	//	{
	//		m_bGetAlarmIn[i] = FALSE;
	//	}
	}
	m_copyalarminsel.SetCurSel(m_alarmin);		
	m_alarminsel.SetCurSel(m_alarmin);
		
	sTemp.Format(_T(NAME_ALARMCFG_ALLOUTPUT));
	m_copyoutsel.AddString(sTemp);
	for(i = 0; i < m_Device->Info.byAlarmOutPortNum; i++)
	{
		sTemp.Format("%d", (1+i));
		sTemp = NAME_ALARMCFG_OUTPUT + sTemp;
		m_alarmoutsel.AddString(sTemp);
		m_copyoutsel.AddString(sTemp);
	//	if(m_bConnectServer)
	//	{
			if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_ALARMOUTCFG, i,&m_alarmoutcfg[i], sizeof(NET_DEV_ALARMOUTCFG), &dwReturned, CONFIG_WAITTIME))
			{
				m_bGetAlarmOut[i] = FALSE;
			//	sTemp.Format("%d 报警输出参数获取失败: NET_DVR_GET_ALARMOUTCFG = %d \n", i, NET_DVR_GetLastError());	
				AfxMessageBox(MSG_ALARMCFG_GETOUTPUTCFGFAILED);
			}
			else
			{
				m_bGetAlarmOut[i] = TRUE;
			}
	//	}
	//	else
	//	{
	//		m_bGetAlarmOut[i] = FALSE;
	//	}
	}
	m_copyoutsel.SetCurSel(m_alarmout);		
	m_alarmoutsel.SetCurSel(m_alarmout);

	if(m_bGetAlarmIn[m_alarmin])
	{
		m_alarmtypesel.SetCurSel(m_alarmincfg[m_alarmin].byAlarmType);
		m_bAlarmIn = m_alarmincfg[m_alarmin].byAlarmInHandle;
		ZeroMemory(cTemp, 100);
		memcpy(cTemp, m_alarmincfg[m_alarmin].sAlarmInName, NAME_LEN);
		m_alarmname.Format("%s", cTemp);
		if(m_bAlarmIn)
		{
			GetDlgItem(IDC_ALARMINSET)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_ALARMINSET)->EnableWindow(FALSE);
		}
		GetDlgItem(IDC_COPYINOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_ALARMINAPPLY)->EnableWindow(TRUE);
	}
	
	if(m_bGetAlarmOut[m_alarmout])
	{
		m_alarmouttimesel.SetCurSel(m_alarmoutcfg[m_alarmout].dwAlarmOutDelay);
		m_alarmdaysel.SetCurSel(0);
		m_copyalarmdaysel.SetCurSel(0);
		m_hour1 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][0].byStartHour;
		m_minute1 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][0].byStartMin;
		m_shour1 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][0].byStopHour;
		m_sminute1 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][0].byStopMin;
		m_hour2 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][1].byStartHour;
		m_minute2 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][1].byStartMin;
		m_shour2 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][1].byStopHour;
		m_sminute2 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][1].byStopMin;
		m_hour3 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][2].byStartHour;
		m_minute3 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][2].byStartMin;
		m_shour3 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][2].byStopHour;
		m_sminute3 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][2].byStopMin;
		m_hour4 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][3].byStartHour;
		m_minute4 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][3].byStartMin;
		m_shour4 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][3].byStopHour;
		m_sminute4 = m_alarmoutcfg[m_alarmout].struAlarmOutTime[0][3].byStopMin;
		GetDlgItem(IDC_COPYOUTOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_ALARMAPPLY)->EnableWindow(TRUE); 
		GetDlgItem(IDC_ALARMOUTDAYAPPLY)->EnableWindow(TRUE); 
	}
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAlarmConfig::OnAlarmapply() 
{
	if (!UpdateData(TRUE))
	{
		return;
	}
	if(!F_TimeTest())
	{
		return;
	}
	int i = m_alarmdaysel.GetCurSel();	
	m_alarmoutcfg[i].dwSize = sizeof(NET_DEV_ALARMOUTCFG);
	m_alarmoutcfg[i].struAlarmOutTime[0][0].byStartHour = m_hour1;
	m_alarmoutcfg[i].struAlarmOutTime[0][0].byStartMin = m_minute1;
	m_alarmoutcfg[i].struAlarmOutTime[0][0].byStopHour = m_shour1;
	m_alarmoutcfg[i].struAlarmOutTime[0][0].byStopMin = m_sminute1;
	m_alarmoutcfg[i].struAlarmOutTime[0][1].byStartHour = m_hour2;
	m_alarmoutcfg[i].struAlarmOutTime[0][1].byStartMin = m_minute2;
	m_alarmoutcfg[i].struAlarmOutTime[0][1].byStopHour = m_shour2;
	m_alarmoutcfg[i].struAlarmOutTime[0][1].byStopMin = m_sminute2;
	m_alarmoutcfg[i].struAlarmOutTime[0][2].byStartHour = m_hour3;
	m_alarmoutcfg[i].struAlarmOutTime[0][2].byStartMin = m_minute3;
	m_alarmoutcfg[i].struAlarmOutTime[0][2].byStopHour = m_shour3;
	m_alarmoutcfg[i].struAlarmOutTime[0][2].byStopMin = m_sminute3;
	m_alarmoutcfg[i].struAlarmOutTime[0][3].byStartHour = m_hour4;
	m_alarmoutcfg[i].struAlarmOutTime[0][3].byStartMin = m_minute4;
	m_alarmoutcfg[i].struAlarmOutTime[0][3].byStopHour = m_shour4;
	m_alarmoutcfg[i].struAlarmOutTime[0][3].byStopMin = m_sminute4;	
	
	m_alarmincfg[m_alarmin].dwSize = sizeof(NET_DEV_ALARMINCFG);
	memcpy(m_alarmincfg[m_alarmin].sAlarmInName, m_alarmname, NAME_LEN);
	m_alarmincfg[m_alarmin].byAlarmInHandle = m_bAlarmIn;
	m_alarmincfg[m_alarmin].byAlarmType = m_alarmtypesel.GetCurSel();
	m_alarmoutcfg[m_alarmout].dwAlarmOutDelay = m_alarmouttimesel.GetCurSel();
	
//	bSetAlarmParams = TRUE;	
	for(i=0; i<(int)m_Device->Info.byAlarmInPortNum; i++)
	{
		
		if(!CLIENT_SetDevConfig(m_Device->LoginID, DEV_SET_ALARMINCFG, i,&m_alarmincfg[i], sizeof(NET_DEV_ALARMINCFG), CONFIG_WAITTIME))
		{
			//sTemp.Format("报警输入%d参数设置失败: NET_DVR_SET_ALARMINCFG = %d \n", i, NET_DVR_GetLastError());
			AfxMessageBox("alarm config save failed!");		
			return;	
		}
	}
	for(i=0; i<(int)m_Device->Info.byAlarmOutPortNum; i++)
	{
		if(!CLIENT_SetDevConfig(m_Device->LoginID, DEV_SET_ALARMOUTCFG, i,&m_alarmoutcfg[i], sizeof(NET_DEV_ALARMOUTCFG), CONFIG_WAITTIME))
		{
		//	sTemp.Format("报警输出%d参数设置失败: NET_DVR_SET_ALARMOUTCFG = %d \n", i, NET_DVR_GetLastError());
			AfxMessageBox("alarm config save failed!");		
			return;	
		}
	}
	bSetAlarmParams = FALSE;
}

BOOL CAlarmConfig::F_TimeTest()
{
	int i, j;
	WORD wStartTime[4], wStopTime[4];
	
	UpdateData(TRUE);
	if( (m_hour1 < 0) || (m_hour1 > 24) || (m_shour1 < 0) || (m_shour1 > 24) ||	\
		(m_hour2 < 0) || (m_hour2 > 24) || (m_shour2 < 0) || (m_shour2 > 24) ||	\
		(m_hour3 < 0) || (m_hour3 > 24) || (m_shour3 < 0) || (m_shour3 > 24) ||	\
		(m_hour4 < 0) || (m_hour4 > 24) || (m_shour4 < 0) || (m_shour4 > 24) )
	{
		AfxMessageBox(MSG_ALARMCFG_HOURERROR);
		return FALSE;
	}
	if( (m_minute1 < 0) || (m_minute1 > 60) || (m_sminute1 < 0) || (m_sminute1 > 60) ||	\
		(m_minute2 < 0) || (m_minute2 > 60) || (m_sminute2 < 0) || (m_sminute2 > 60) ||	\
		(m_minute3 < 0) || (m_minute3 > 60) || (m_sminute3 < 0) || (m_sminute3 > 60) ||	\
		(m_minute4 < 0) || (m_minute4 > 60) || (m_sminute4 < 0) || (m_sminute4 > 60) )
	{
		AfxMessageBox(MSG_ALARMCFG_MINUTEERROR);
		return FALSE;
	}
	wStartTime[0] = m_hour1 * 100 + m_minute1;
	wStartTime[1] = m_hour2 * 100 + m_minute2;
	wStartTime[2] = m_hour3 * 100 + m_minute3;
	wStartTime[3] = m_hour4 * 100 + m_minute4;
	wStopTime[0] = m_shour1 * 100 + m_sminute1;
	wStopTime[1] = m_shour2 * 100 + m_sminute2;
	wStopTime[2] = m_shour3 * 100 + m_sminute3;
	wStopTime[3] = m_shour4 * 100 + m_sminute4;
	for(i = 0; i < 4; i++)
	{
		if( (wStartTime[i] > 2400) || (wStopTime[i] > 2400) )
		{
			AfxMessageBox(MSG_ALARMCFG_TIMEILLEGAL);
			return FALSE;
		}
		if(wStartTime[i] > wStopTime[i])
		{
			AfxMessageBox(MSG_ALARMCFG_TIMEILLEGAL2);
			return FALSE;
		}
		for(j = 0; j < 4; j++)
		{
			if (i == j)
			{
				continue;
			}
			if( (wStartTime[i] > wStartTime[j]) && (wStartTime[i] < wStopTime[j]) ||	\
				(wStopTime[i] > wStartTime[j]) && (wStopTime[i] < wStopTime[j]) )
			{
				AfxMessageBox(MSG_ALARMCFG_TIMEILLEGAL3);
				return FALSE;
			}
		}
	}
	return TRUE;
}

void CAlarmConfig::OnSelchangeAlarminsel() 
{
	char cTemp[100];
	m_alarmin = m_alarminsel.GetCurSel();
	ZeroMemory(cTemp, 100);
	memcpy(cTemp, m_alarmincfg[m_alarmin].sAlarmInName, NAME_LEN);
	m_alarmname.Format("%s", cTemp);
	m_alarmtypesel.SetCurSel(m_alarmincfg[m_alarmin].byAlarmType);
	m_bAlarmIn = m_alarmincfg[m_alarmin].byAlarmInHandle;
	if(m_bAlarmIn)
	{
		GetDlgItem(IDC_ALARMINSET)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_ALARMINSET)->EnableWindow(FALSE);
	}
	UpdateData(FALSE);	
}

void CAlarmConfig::OnAlarmoutrestore() 
{
	OnInitDialog();
}
