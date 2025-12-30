// ConfigAlarmAlarm.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigAlarmAlarm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmAlarm dialog


CConfigAlarmAlarm::CConfigAlarmAlarm(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigAlarmAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigAlarmAlarm)
	//}}AFX_DATA_INIT
	m_pDev = 0;
	memset(m_alarmCFG, 0, DH_MAX_ALARM_IN_NUM*sizeof(DH_ALARMIN_CFG));
	m_bReady = FALSE;
	m_almIdx = -1;
	m_weekIdx = -1;
	m_ptzchlIdx = -1;

	m_dontsave  = 0;
}


void CConfigAlarmAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigAlarmAlarm)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigAlarmAlarm, CDialog)
	//{{AFX_MSG_MAP(CConfigAlarmAlarm)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_ALARMIN, OnSelchangeComboAlarmin)
	ON_CBN_SELCHANGE(IDC_COMBO_ALARMWEEK, OnSelchangeComboAlarmweek)
	ON_CBN_SELCHANGE(IDC_COMBO_ALARMPTZCHL, OnSelchangeComboAlarmptzchl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmAlarm message handlers

void CConfigAlarmAlarm::SetAlarmInCFG(DH_ALARMIN_CFG *cfg)
{
	if (!cfg)
	{
		return;
	}
	memcpy(m_alarmCFG, cfg, DH_MAX_ALARM_IN_NUM*sizeof(DH_ALARMIN_CFG));

	DWORD dwActMask = m_alarmCFG[0].struHandle.dwActionMask;
	//sentence if some functions enabled
	GetDlgItem(IDC_CHECK_UPLOAD)->EnableWindow((dwActMask & DH_ALARM_UPLOAD)?TRUE:FALSE);
	GetDlgItem(IDC_CHECK_LOCALTIP)->EnableWindow((dwActMask & DH_ALARM_TIP)?TRUE:FALSE);
	GetDlgItem(IDC_CHECK_LOCALTOUR)->EnableWindow((dwActMask & DH_ALARM_TOUR)?TRUE:FALSE);
	GetDlgItem(IDC_CHECK_SENDMAIL)->EnableWindow((dwActMask & DH_ALARM_MAIL)?TRUE:FALSE);

	//sentence if ptz enabled
	BOOL bPTZEn = (dwActMask & DH_ALARM_PTZ)?TRUE:FALSE;
	GetDlgItem(IDC_CHECK_PTZ)->EnableWindow(bPTZEn);
	GetDlgItem(IDC_COMBO_ALARMPTZCHL)->EnableWindow(bPTZEn);
	GetDlgItem(IDC_COMBO_ALARMPTZ)->EnableWindow(bPTZEn);
	GetDlgItem(IDC_EDIT_PTZNO)->EnableWindow(bPTZEn);
	
	//sentence if record enabled
	BOOL bRecEn = (dwActMask & DH_ALARM_RECORD)?TRUE:FALSE;
	GetDlgItem(IDC_CHECK_RECORD)->EnableWindow(bRecEn);
	for (int i = 0; i < 16; i++)
	{
		GetDlgItem(IDC_CHECK_RECCH1+i)->EnableWindow(bRecEn);
	}

	//sentence if alarm output enabled
	BOOL bOutEn = (dwActMask & DH_ALARM_OUT)?TRUE:FALSE;
	GetDlgItem(IDC_CHECK_OUTPUT)->EnableWindow(bOutEn);
	for (int j = 0; j < 16; j++)
	{
		GetDlgItem(IDC_CHECK_ALMOP1+j)->EnableWindow(bOutEn);
	}

	m_bReady = TRUE;
}

void CConfigAlarmAlarm::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

void CConfigAlarmAlarm::ShowConfig()
{
	if (!m_bReady)
	{
		return;
	}

	if (((CComboBox*)GetDlgItem(IDC_COMBO_ALARMIN))->GetCount() < 0)
	{
		return;
	}

	//show
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMIN))->SetCurSel(0);
	OnSelchangeComboAlarmin();
}

void CConfigAlarmAlarm::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	ShowConfig();
}

BOOL CConfigAlarmAlarm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	if (!m_pDev)
	{
		return TRUE;
	}

	CString strAlm;
	int i = 0;
	for (i = 0; i < m_pDev->Info.nAlarmInPortNum; i++)
	{
		strAlm.Format("Alarm %d", i+1);
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMIN))->InsertString(i, strAlm);
	}
	for (int j = 15; j >= m_pDev->Info.nAlarmOutPortNum; j--)
	{
		GetDlgItem(IDC_CHECK_ALMOP1+j)->ShowWindow(SW_HIDE);
	}
	for (int k = 15; k >= m_pDev->Info.nChanNum; k--)
	{
		GetDlgItem(IDC_CHECK_RECCH1+k)->ShowWindow(SW_HIDE);
	}
	
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(0, ConvertString("Sunday"));
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(1, ConvertString("Monday"));
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(2, ConvertString("Tuesday"));
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(3, ConvertString("Wedensday"));
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(4, ConvertString("Thursday"));
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(5, ConvertString("Friday"));
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->InsertString(6, ConvertString("Saturday"));
	
	CString strChl;
	for (int l = 0; l < m_pDev->Info.nChanNum; l++)
	{
		strChl.Format(ConvertString("channel %d"), i+1);
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))->InsertString(i, strChl);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigAlarmAlarm::OnSelchangeComboAlarmin() 
{
	if (!m_bReady)
	{
		return;
	}
	int i,j,k;

	//store current config
	if (m_almIdx < 0)
	{
		goto skipsave;
	}
	m_alarmCFG[m_almIdx].byAlarmEn = ((CButton*)GetDlgItem(IDC_CHECK_ALARMENABLE))->GetCheck();
	m_alarmCFG[m_almIdx].byAlarmType = ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMTYPE))->GetCurSel();
	
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (i = 0; i < 6; i++)
		{
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}
	
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag = 0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_UPLOAD))->GetCheck()?DH_ALARM_UPLOAD:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTIP))->GetCheck()?DH_ALARM_TIP:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTOUR))->GetCheck()?DH_ALARM_TOUR:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_SENDMAIL))->GetCheck()?DH_ALARM_MAIL:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_PTZ))->GetCheck()?DH_ALARM_PTZ:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_RECORD))->GetCheck()?DH_ALARM_RECORD:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_OUTPUT))->GetCheck()?DH_ALARM_OUT:0;

	for (j = 0; j < m_pDev->Info.nAlarmOutPortNum; j++)
	{
		m_alarmCFG[m_almIdx].struHandle.byRelAlarmOut[j] = ((CButton*)GetDlgItem(IDC_CHECK_ALMOP1+j))->GetCheck();
	}

	for (k = 0; k < m_pDev->Info.nChanNum; k++)
	{
		m_alarmCFG[m_almIdx].struHandle.byRecordChannel[k] = ((CButton*)GetDlgItem(IDC_CHECK_RECCH1+k))->GetCheck();
	}

	if (m_ptzchlIdx >= 0)
	{
		m_alarmCFG[m_almIdx].struHandle.struPtzLink[m_ptzchlIdx].iType = ((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->GetCurSel();
		m_alarmCFG[m_almIdx].struHandle.struPtzLink[m_ptzchlIdx].iValue = GetDlgItemInt(IDC_EDIT_PTZNO);
	}

skipsave:
	//show new ones
	int almIdx = ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMIN))->GetCurSel();
	if (almIdx < 0 || ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->GetCount() < 0)
	{
		return;
	}

	m_almIdx = almIdx;
	((CButton*)GetDlgItem(IDC_CHECK_ALARMENABLE))->SetCheck(m_alarmCFG[almIdx].byAlarmEn?1:0);
	((CComboBox*)GetDlgItem(IDC_COMBO_ALARMTYPE))->SetCurSel(m_alarmCFG[almIdx].byAlarmType);
	
	if (((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->GetCount() > 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->SetCurSel(0);
		m_weekIdx = -1; //prevent from unwanted saving
		OnSelchangeComboAlarmweek() ;
	}
	
	((CButton*)GetDlgItem(IDC_CHECK_UPLOAD))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_UPLOAD)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_LOCALTIP))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_TIP)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_LOCALTOUR))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_TOUR)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_SENDMAIL))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_MAIL)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_PTZ))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_PTZ)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_RECORD))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_RECORD)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_OUTPUT))->SetCheck((m_alarmCFG[almIdx].struHandle.dwActionFlag&DH_ALARM_OUT)?1:0);
	
	for (j = 0; j < m_pDev->Info.nAlarmOutPortNum; j++)
	{
		((CButton*)GetDlgItem(IDC_CHECK_ALMOP1+j))->SetCheck(m_alarmCFG[almIdx].struHandle.byRelAlarmOut[j]?1:0);
	}

	for (k = 0; k < m_pDev->Info.nChanNum; k++)
	{
		((CButton*)GetDlgItem(IDC_CHECK_RECCH1+k))->SetCheck(m_alarmCFG[almIdx].struHandle.byRecordChannel[k]?1:0);
	}

	if (((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))-> GetCount() > 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))-> SetCurSel(0);
		m_ptzchlIdx = -1; //prevent from unwanted saving
		OnSelchangeComboAlarmptzchl();
	}
}

void CConfigAlarmAlarm::OnSelchangeComboAlarmweek() 
{
	if (!m_bReady)
	{
		return;
	}

	if (m_almIdx < 0)
	{
		return;
	}

	//store current time sectors
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}

	//show new time sectors
	int weekIdx = ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->GetCurSel();
	if (weekIdx < 0)
	{
		return;
	}
	m_weekIdx = weekIdx;

	int timeCtrlIdx = 0;
	for (int i = 0; i < 6; i++)
	{
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_alarmCFG[m_almIdx].stSect[weekIdx][i].iBeginHour);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_alarmCFG[m_almIdx].stSect[weekIdx][i].iBeginMin);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_alarmCFG[m_almIdx].stSect[weekIdx][i].iBeginSec);
		timeCtrlIdx++;
		
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_alarmCFG[m_almIdx].stSect[weekIdx][i].iEndHour);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_alarmCFG[m_almIdx].stSect[weekIdx][i].iEndMin);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_alarmCFG[m_almIdx].stSect[weekIdx][i].iEndSec);
		timeCtrlIdx++;
	}
}

void CConfigAlarmAlarm::OnSelchangeComboAlarmptzchl() 
{
	if (!m_bReady || m_almIdx < 0)
	{
		return;
	}

	if (m_ptzchlIdx >= 0)
	{
		m_alarmCFG[m_almIdx].struHandle.struPtzLink[m_ptzchlIdx].iType = ((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->GetCurSel();
		m_alarmCFG[m_almIdx].struHandle.struPtzLink[m_ptzchlIdx].iValue = GetDlgItemInt(IDC_EDIT_PTZNO);
	}

	int ptzchlIdx = ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))-> GetCount();
	if (ptzchlIdx < 0)
	{
		return;
	}
	m_ptzchlIdx = ptzchlIdx;
	((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->SetCurSel(m_alarmCFG[m_almIdx].struHandle.struPtzLink[ptzchlIdx].iType);
	SetDlgItemInt(IDC_EDIT_PTZNO,m_alarmCFG[m_almIdx].struHandle.struPtzLink[ptzchlIdx].iValue);
}

void CConfigAlarmAlarm::GetAlarmInCFG(DH_ALARMIN_CFG *cfg)
{
	if (!cfg || !m_bReady || m_almIdx < 0)
	{
		return;
	}

	m_alarmCFG[m_almIdx].byAlarmEn = ((CButton*)GetDlgItem(IDC_CHECK_ALARMENABLE))->GetCheck();
	m_alarmCFG[m_almIdx].byAlarmType = ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMTYPE))->GetCurSel();
	
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_alarmCFG[m_almIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}
	
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag = 0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_UPLOAD))->GetCheck()?DH_ALARM_UPLOAD:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTIP))->GetCheck()?DH_ALARM_TIP:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTOUR))->GetCheck()?DH_ALARM_TOUR:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_SENDMAIL))->GetCheck()?DH_ALARM_MAIL:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_PTZ))->GetCheck()?DH_ALARM_PTZ:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_RECORD))->GetCheck()?DH_ALARM_RECORD:0;
	m_alarmCFG[m_almIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_OUTPUT))->GetCheck()?DH_ALARM_OUT:0;

	for (int j = 0; j < m_pDev->Info.nAlarmOutPortNum; j++)
	{
		m_alarmCFG[m_almIdx].struHandle.byRelAlarmOut[j] = ((CButton*)GetDlgItem(IDC_CHECK_ALMOP1+j))->GetCheck();
	}

	for (int k = 0; k < m_pDev->Info.nChanNum; k++)
	{
		m_alarmCFG[m_almIdx].struHandle.byRecordChannel[k] = ((CButton*)GetDlgItem(IDC_CHECK_RECCH1+k))->GetCheck();
	}

	if (m_ptzchlIdx >= 0)
	{
		m_alarmCFG[m_almIdx].struHandle.struPtzLink[m_ptzchlIdx].iType = ((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->GetCurSel();
		m_alarmCFG[m_almIdx].struHandle.struPtzLink[m_ptzchlIdx].iValue = GetDlgItemInt(IDC_EDIT_PTZNO);
	}

	memcpy(cfg, m_alarmCFG, DH_MAX_ALARM_IN_NUM*sizeof(DH_ALARMIN_CFG));
}

void CConfigAlarmAlarm::UndoAll()
{
	m_bReady = FALSE;
	m_almIdx = -1;
	m_weekIdx = -1;
	m_ptzchlIdx = -1;
}
