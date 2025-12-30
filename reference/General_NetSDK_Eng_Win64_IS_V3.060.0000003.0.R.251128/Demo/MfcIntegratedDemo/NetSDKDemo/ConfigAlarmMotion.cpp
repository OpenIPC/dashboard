// ConfigAlarmMotion.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigAlarmMotion.h"
#include "ConfigAlarmMotionArea.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotion dialog


CConfigAlarmMotion::CConfigAlarmMotion(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigAlarmMotion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigAlarmMotion)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDev = 0;
	memset(m_motionCFG, 0, DH_MAX_VIDEO_IN_NUM*sizeof(DH_MOTION_DETECT_CFG));
	m_bReady = FALSE;
	m_chlIdx = -1;
	m_weekIdx = -1;
	m_ptzchlIdx = -1;

	m_dontsave = 0;
}

void CConfigAlarmMotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigAlarmMotion)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigAlarmMotion, CDialog)
	//{{AFX_MSG_MAP(CConfigAlarmMotion)
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEOCHL, OnSelchangeComboVideochl)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_ALARMWEEK, OnSelchangeComboAlarmweek)
	ON_CBN_SELCHANGE(IDC_COMBO_ALARMPTZCHL, OnSelchangeComboAlarmptzchl)
	ON_BN_CLICKED(IDC_BTN_AREA, OnBtnArea)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotion message handlers

void CConfigAlarmMotion::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

void CConfigAlarmMotion::SetMotionCFG(DH_MOTION_DETECT_CFG *cfg)
{
	if (!cfg)
	{
		return;
	}

	memcpy(m_motionCFG, cfg, DH_MAX_VIDEO_IN_NUM*sizeof(DH_MOTION_DETECT_CFG));

	DWORD dwActMask = m_motionCFG[0].struHandle.dwActionMask;
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

void CConfigAlarmMotion::ShowConfig()
{
	if (!m_bReady)
	{
		return;
	}

	if (((CComboBox*)GetDlgItem(IDC_COMBO_VIDEOCHL))->GetCount() < 0)
	{
		return;
	}

	//show
	((CComboBox*)GetDlgItem(IDC_COMBO_VIDEOCHL))->SetCurSel(0);
	OnSelchangeComboVideochl();
}

void CConfigAlarmMotion::OnSelchangeComboVideochl() 
{
	if (!m_bReady)
	{
		return;
	}
	int i,j,k;

	//save current config
	if (m_chlIdx < 0)
	{
		goto skipsave;
	}
	m_motionCFG[m_chlIdx].byMotionEn = ((CButton*)GetDlgItem(IDC_CHECK_MOTIONENABLE))->GetCheck();
	m_motionCFG[m_chlIdx].wSenseLevel = GetDlgItemInt(IDC_EDIT_SENSELVL);
	
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (i = 0; i < 6; i++)
		{
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}
	
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag = 0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_UPLOAD))->GetCheck()?DH_ALARM_UPLOAD:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTIP))->GetCheck()?DH_ALARM_TIP:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTOUR))->GetCheck()?DH_ALARM_TOUR:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_SENDMAIL))->GetCheck()?DH_ALARM_MAIL:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_PTZ))->GetCheck()?DH_ALARM_PTZ:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_RECORD))->GetCheck()?DH_ALARM_RECORD:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_OUTPUT))->GetCheck()?DH_ALARM_OUT:0;

	for (j = 0; j < m_pDev->Info.nAlarmOutPortNum; j++)
	{
		m_motionCFG[m_chlIdx].struHandle.byRelAlarmOut[j] = ((CButton*)GetDlgItem(IDC_CHECK_ALMOP1+j))->GetCheck();
	}

	for (k = 0; k < m_pDev->Info.nChanNum; k++)
	{
		m_motionCFG[m_chlIdx].struHandle.byRecordChannel[k] = ((CButton*)GetDlgItem(IDC_CHECK_RECCH1+k))->GetCheck();
	}

	if (m_ptzchlIdx >= 0)
	{
		m_motionCFG[m_chlIdx].struHandle.struPtzLink[m_ptzchlIdx].iType = ((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->GetCurSel();
		m_motionCFG[m_chlIdx].struHandle.struPtzLink[m_ptzchlIdx].iValue = GetDlgItemInt(IDC_EDIT_PTZNO);
	}
	
skipsave:
	//show new ones
	int chlIdx = ((CComboBox*)GetDlgItem(IDC_COMBO_VIDEOCHL))->GetCurSel();
	if (chlIdx < 0 || ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->GetCount() < 0)
	{
		return;
	}

	m_chlIdx = chlIdx;
	((CButton*)GetDlgItem(IDC_CHECK_MOTIONENABLE))->SetCheck(m_motionCFG[chlIdx].byMotionEn?1:0);
	SetDlgItemInt(IDC_EDIT_SENSELVL, m_motionCFG[chlIdx].wSenseLevel);
	
	if (((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->GetCount() > 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMWEEK))->SetCurSel(0);
		m_weekIdx = -1; //prevent from unwanted saving
		OnSelchangeComboAlarmweek() ;
	}
	
	((CButton*)GetDlgItem(IDC_CHECK_UPLOAD))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_UPLOAD)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_LOCALTIP))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_TIP)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_LOCALTOUR))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_TOUR)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_SENDMAIL))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_MAIL)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_PTZ))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_PTZ)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_RECORD))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_RECORD)?1:0);
	((CButton*)GetDlgItem(IDC_CHECK_OUTPUT))->SetCheck((m_motionCFG[chlIdx].struHandle.dwActionFlag&DH_ALARM_OUT)?1:0);
	
	for (j = 0; j < m_pDev->Info.nAlarmOutPortNum; j++)
	{
		((CButton*)GetDlgItem(IDC_CHECK_ALMOP1+j))->SetCheck(m_motionCFG[chlIdx].struHandle.byRelAlarmOut[j]?1:0);
	}

	for (k = 0; k < m_pDev->Info.nChanNum; k++)
	{
		((CButton*)GetDlgItem(IDC_CHECK_RECCH1+k))->SetCheck(m_motionCFG[chlIdx].struHandle.byRecordChannel[k]?1:0);
	}

	if (((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))-> GetCount() > 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))-> SetCurSel(0);
		m_ptzchlIdx = -1; //prevent from unwanted saving
		OnSelchangeComboAlarmptzchl();
	}
}

BOOL CConfigAlarmMotion::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	if (!m_pDev)
	{
		return TRUE;
	}

	CString strMotion;
	int i = 0;
	for (i = 0; i < m_pDev->Info.nChanNum; i++)
	{
		strMotion.Format(ConvertString("Channel %d"), i+1);
		((CComboBox*)GetDlgItem(IDC_COMBO_VIDEOCHL))->InsertString(i, strMotion);
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
		strChl.Format("channel %d", i+1);
		((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))->InsertString(i, strChl);
	}	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigAlarmMotion::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	ShowConfig();
}

void CConfigAlarmMotion::OnSelchangeComboAlarmweek() 
{
	if (!m_bReady || m_chlIdx < 0)
	{
		return;
	}

	//store current time sectors
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
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
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_motionCFG[m_chlIdx].stSect[weekIdx][i].iBeginHour);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_motionCFG[m_chlIdx].stSect[weekIdx][i].iBeginMin);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_motionCFG[m_chlIdx].stSect[weekIdx][i].iBeginSec);
		timeCtrlIdx++;
		
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_motionCFG[m_chlIdx].stSect[weekIdx][i].iEndHour);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_motionCFG[m_chlIdx].stSect[weekIdx][i].iEndMin);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_motionCFG[m_chlIdx].stSect[weekIdx][i].iEndSec);
		timeCtrlIdx++;
	}
}

void CConfigAlarmMotion::OnSelchangeComboAlarmptzchl() 
{
	if (!m_bReady || m_chlIdx < 0)
	{
		return;
	}

	if (m_ptzchlIdx >= 0)
	{
		m_motionCFG[m_chlIdx].struHandle.struPtzLink[m_ptzchlIdx].iType = ((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->GetCurSel();
		m_motionCFG[m_chlIdx].struHandle.struPtzLink[m_ptzchlIdx].iValue = GetDlgItemInt(IDC_EDIT_PTZNO);
	}

	int ptzchlIdx = ((CComboBox*)GetDlgItem(IDC_COMBO_ALARMPTZCHL))-> GetCount();
	if (ptzchlIdx < 0)
	{
		return;
	}
	m_ptzchlIdx = ptzchlIdx;
	((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->SetCurSel(m_motionCFG[m_chlIdx].struHandle.struPtzLink[ptzchlIdx].iType);
	SetDlgItemInt(IDC_EDIT_PTZNO,m_motionCFG[m_chlIdx].struHandle.struPtzLink[ptzchlIdx].iValue);
}

void CConfigAlarmMotion::OnBtnArea() 
{
	CConfigAlarmMotionArea areaDlg;
	areaDlg.SetArea((BYTE*)m_motionCFG[m_chlIdx].byDetected);
	if (IDOK == areaDlg.DoModal())
	{
		memcpy(m_motionCFG[m_chlIdx].byDetected, areaDlg.GetArea(), DH_MOTION_ROW*DH_MOTION_COL);
	}
}

void CConfigAlarmMotion::GetMotionCFG(DH_MOTION_DETECT_CFG *cfg)
{
	if (!cfg || !m_bReady || m_chlIdx < 0)
	{
		return;
	}

	m_motionCFG[m_chlIdx].byMotionEn = ((CButton*)GetDlgItem(IDC_CHECK_MOTIONENABLE))->GetCheck();
	m_motionCFG[m_chlIdx].wSenseLevel = GetDlgItemInt(IDC_EDIT_SENSELVL);
	
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_motionCFG[m_chlIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}
	
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag = 0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_UPLOAD))->GetCheck()?DH_ALARM_UPLOAD:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTIP))->GetCheck()?DH_ALARM_TIP:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_LOCALTOUR))->GetCheck()?DH_ALARM_TOUR:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_SENDMAIL))->GetCheck()?DH_ALARM_MAIL:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_PTZ))->GetCheck()?DH_ALARM_PTZ:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_RECORD))->GetCheck()?DH_ALARM_RECORD:0;
	m_motionCFG[m_chlIdx].struHandle.dwActionFlag |= ((CButton*)GetDlgItem(IDC_CHECK_OUTPUT))->GetCheck()?DH_ALARM_OUT:0;

	for (int j = 0; j < m_pDev->Info.nAlarmOutPortNum; j++)
	{
		m_motionCFG[m_chlIdx].struHandle.byRelAlarmOut[j] = ((CButton*)GetDlgItem(IDC_CHECK_ALMOP1+j))->GetCheck();
	}

	for (int k = 0; k < m_pDev->Info.nChanNum; k++)
	{
		m_motionCFG[m_chlIdx].struHandle.byRecordChannel[k] = ((CButton*)GetDlgItem(IDC_CHECK_RECCH1+k))->GetCheck();
	}

	if (m_ptzchlIdx >= 0)
	{
		m_motionCFG[m_chlIdx].struHandle.struPtzLink[m_ptzchlIdx].iType = ((CComboBox*)GetDlgItem(IDC_CHECK_PTZ))->GetCurSel();
		m_motionCFG[m_chlIdx].struHandle.struPtzLink[m_ptzchlIdx].iValue = GetDlgItemInt(IDC_EDIT_PTZNO);
	}

	memcpy(cfg, m_motionCFG, DH_MAX_ALARM_IN_NUM*sizeof(DH_MOTION_DETECT_CFG));
}

void CConfigAlarmMotion::UndoAll()
{
	m_bReady = FALSE;
	m_chlIdx = -1;
	m_weekIdx = -1;
	m_ptzchlIdx = -1;
}
